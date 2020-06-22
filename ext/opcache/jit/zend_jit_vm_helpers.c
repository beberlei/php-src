/*
   +----------------------------------------------------------------------+
   | Zend JIT                                                             |
   +----------------------------------------------------------------------+
   | Copyright (c) The PHP Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Dmitry Stogov <dmitry@php.net>                              |
   |          Xinchen Hui <laruence@php.net>                              |
   +----------------------------------------------------------------------+
*/

#include "Zend/zend_execute.h"
#include "Zend/zend_exceptions.h"
#include "Zend/zend_vm.h"
#include "Zend/zend_closures.h"
#include "Zend/zend_constants.h"
#include "Zend/zend_API.h"

#include <ZendAccelerator.h>
#include "Optimizer/zend_func_info.h"
#include "Optimizer/zend_call_graph.h"
#include "zend_jit.h"
#include "zend_jit_x86.h"
#include "zend_jit_internal.h"

#ifdef HAVE_GCC_GLOBAL_REGS
# pragma GCC diagnostic ignored "-Wvolatile-register-var"
# if defined(__x86_64__)
register zend_execute_data* volatile execute_data __asm__("%r14");
register const zend_op* volatile opline __asm__("%r15");
# else
register zend_execute_data* volatile execute_data __asm__("%esi");
register const zend_op* volatile opline __asm__("%edi");
# endif
# pragma GCC diagnostic warning "-Wvolatile-register-var"
#endif

ZEND_OPCODE_HANDLER_RET ZEND_FASTCALL zend_jit_leave_nested_func_helper(uint32_t call_info EXECUTE_DATA_DC)
{
	zend_execute_data *old_execute_data;

	if (UNEXPECTED(call_info & ZEND_CALL_HAS_SYMBOL_TABLE)) {
		zend_clean_and_cache_symbol_table(EX(symbol_table));
	}

	zend_vm_stack_free_extra_args_ex(call_info, execute_data);
	if (UNEXPECTED(call_info & ZEND_CALL_RELEASE_THIS)) {
		OBJ_RELEASE(Z_OBJ(execute_data->This));
	} else if (UNEXPECTED(call_info & ZEND_CALL_CLOSURE)) {
		OBJ_RELEASE(ZEND_CLOSURE_OBJECT(EX(func)));
	}
	if (UNEXPECTED(call_info & ZEND_CALL_HAS_EXTRA_NAMED_PARAMS)) {
		zend_array_destroy(EX(extra_named_params));
	}

	old_execute_data = execute_data;
	execute_data = EX(prev_execute_data);
	zend_vm_stack_free_call_frame_ex(call_info, old_execute_data);

	if (UNEXPECTED(EG(exception) != NULL)) {
		const zend_op *old_opline = EX(opline);
		zend_throw_exception_internal(NULL);
		if (old_opline->result_type != IS_UNDEF) {
			zval_ptr_dtor(EX_VAR(old_opline->result.var));
		}
#ifndef HAVE_GCC_GLOBAL_REGS
		return 2; // ZEND_VM_LEAVE
#endif
	} else {
		EX(opline)++;
#ifdef HAVE_GCC_GLOBAL_REGS
		opline = EX(opline);
#else
		return 2; // ZEND_VM_LEAVE
#endif
	}
}

ZEND_OPCODE_HANDLER_RET ZEND_FASTCALL zend_jit_leave_top_func_helper(uint32_t call_info EXECUTE_DATA_DC)
{
	if (UNEXPECTED(call_info & (ZEND_CALL_HAS_SYMBOL_TABLE|ZEND_CALL_FREE_EXTRA_ARGS))) {
		if (UNEXPECTED(call_info & ZEND_CALL_HAS_SYMBOL_TABLE)) {
			zend_clean_and_cache_symbol_table(EX(symbol_table));
		}
		zend_vm_stack_free_extra_args_ex(call_info, execute_data);
	}
	if (UNEXPECTED(call_info & ZEND_CALL_CLOSURE)) {
		OBJ_RELEASE(ZEND_CLOSURE_OBJECT(EX(func)));
	}
	execute_data = EG(current_execute_data);
#ifdef HAVE_GCC_GLOBAL_REGS
	opline = zend_jit_halt_op;
#else
	return -1; // ZEND_VM_RETURN
#endif
}

ZEND_OPCODE_HANDLER_RET ZEND_FASTCALL zend_jit_leave_func_helper(uint32_t call_info EXECUTE_DATA_DC)
{
	if (call_info & ZEND_CALL_TOP) {
		ZEND_OPCODE_TAIL_CALL_EX(zend_jit_leave_top_func_helper, call_info);
	} else {
		ZEND_OPCODE_TAIL_CALL_EX(zend_jit_leave_nested_func_helper, call_info);
	}
}

void ZEND_FASTCALL zend_jit_copy_extra_args_helper(EXECUTE_DATA_D)
{
	zend_op_array *op_array = &EX(func)->op_array;

	if (EXPECTED(!(op_array->fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE))) {
		uint32_t first_extra_arg = op_array->num_args;
		uint32_t num_args = EX_NUM_ARGS();
		zval *end, *src, *dst;
		uint32_t type_flags = 0;

		if (EXPECTED((op_array->fn_flags & ZEND_ACC_HAS_TYPE_HINTS) == 0)) {
			/* Skip useless ZEND_RECV and ZEND_RECV_INIT opcodes */
#ifdef HAVE_GCC_GLOBAL_REGS
			opline += first_extra_arg;
#endif
		}

		/* move extra args into separate array after all CV and TMP vars */
		end = EX_VAR_NUM(first_extra_arg - 1);
		src = end + (num_args - first_extra_arg);
		dst = src + (op_array->last_var + op_array->T - first_extra_arg);
		if (EXPECTED(src != dst)) {
			do {
				type_flags |= Z_TYPE_INFO_P(src);
				ZVAL_COPY_VALUE(dst, src);
				ZVAL_UNDEF(src);
				src--;
				dst--;
			} while (src != end);
			if (type_flags & (IS_TYPE_REFCOUNTED << Z_TYPE_FLAGS_SHIFT)) {
				ZEND_ADD_CALL_FLAG(execute_data, ZEND_CALL_FREE_EXTRA_ARGS);
			}
		} else {
			do {
				if (Z_REFCOUNTED_P(src)) {
					ZEND_ADD_CALL_FLAG(execute_data, ZEND_CALL_FREE_EXTRA_ARGS);
					break;
				}
				src--;
			} while (src != end);
		}
	}
}

zend_bool ZEND_FASTCALL zend_jit_deprecated_helper(OPLINE_D)
{
	zend_execute_data *call = (zend_execute_data *) opline;
	zend_function *fbc = call->func;

	zend_deprecated_function(fbc);

	if (EG(exception)) {
#ifndef HAVE_GCC_GLOBAL_REGS
		zend_execute_data *execute_data = EG(current_execute_data);
#endif
		const zend_op *opline = EG(opline_before_exception);
		if (RETURN_VALUE_USED(opline)) {
			ZVAL_UNDEF(EX_VAR(opline->result.var));
		}

		zend_vm_stack_free_args(call);

		if (UNEXPECTED(ZEND_CALL_INFO(call) & ZEND_CALL_RELEASE_THIS)) {
			OBJ_RELEASE(Z_OBJ(call->This));
		}

		zend_vm_stack_free_call_frame(call);
		return 0;
	}
	return 1;
}

ZEND_OPCODE_HANDLER_RET ZEND_FASTCALL zend_jit_profile_helper(ZEND_OPCODE_HANDLER_ARGS)
{
	zend_op_array *op_array = (zend_op_array*)EX(func);
	zend_jit_op_array_extension *jit_extension = (zend_jit_op_array_extension*)ZEND_FUNC_INFO(op_array);
	zend_vm_opcode_handler_t handler = (zend_vm_opcode_handler_t) jit_extension->orig_handler;
	++*(uintptr_t*)(EX(run_time_cache) + zend_jit_profile_counter_rid);
	++zend_jit_profile_counter;
	ZEND_OPCODE_TAIL_CALL(handler);
}

ZEND_OPCODE_HANDLER_RET ZEND_FASTCALL zend_jit_func_counter_helper(ZEND_OPCODE_HANDLER_ARGS)
{
	zend_jit_op_array_hot_extension *jit_extension =
		(zend_jit_op_array_hot_extension*)ZEND_FUNC_INFO(&EX(func)->op_array);
#ifndef HAVE_GCC_GLOBAL_REGS
	const zend_op *opline = EX(opline);
#endif

	*(jit_extension->counter) -= ((ZEND_JIT_COUNTER_INIT + JIT_G(hot_func) - 1) / JIT_G(hot_func));

	if (UNEXPECTED(*(jit_extension->counter) <= 0)) {
		*(jit_extension->counter) = ZEND_JIT_COUNTER_INIT;
		zend_jit_hot_func(execute_data, opline);
		ZEND_OPCODE_RETURN();
	} else {
		zend_vm_opcode_handler_t handler = (zend_vm_opcode_handler_t)jit_extension->orig_handlers[opline - EX(func)->op_array.opcodes];
		ZEND_OPCODE_TAIL_CALL(handler);
	}
}

ZEND_OPCODE_HANDLER_RET ZEND_FASTCALL zend_jit_loop_counter_helper(ZEND_OPCODE_HANDLER_ARGS)
{
	zend_jit_op_array_hot_extension *jit_extension =
		(zend_jit_op_array_hot_extension*)ZEND_FUNC_INFO(&EX(func)->op_array);
#ifndef HAVE_GCC_GLOBAL_REGS
	const zend_op *opline = EX(opline);
#endif

	*(jit_extension->counter) -= ((ZEND_JIT_COUNTER_INIT + JIT_G(hot_loop) - 1) / JIT_G(hot_loop));

	if (UNEXPECTED(*(jit_extension->counter) <= 0)) {
		*(jit_extension->counter) = ZEND_JIT_COUNTER_INIT;
		zend_jit_hot_func(execute_data, opline);
		ZEND_OPCODE_RETURN();
	} else {
		zend_vm_opcode_handler_t handler = (zend_vm_opcode_handler_t)jit_extension->orig_handlers[opline - EX(func)->op_array.opcodes];
		ZEND_OPCODE_TAIL_CALL(handler);
	}
}

static zend_always_inline int _zend_quick_get_constant(
		const zval *key, uint32_t flags, int check_defined_only)
{
#ifndef HAVE_GCC_GLOBAL_REGS
	zend_execute_data *execute_data = EG(current_execute_data);
#endif
	const zend_op *opline = EX(opline);
	zval *zv;
	zend_constant *c = NULL;

	/* null/true/false are resolved during compilation, so don't check for them here. */
	zv = zend_hash_find_ex(EG(zend_constants), Z_STR_P(key), 1);
	if (zv) {
		c = (zend_constant*)Z_PTR_P(zv);
	} else if (flags & IS_CONSTANT_UNQUALIFIED_IN_NAMESPACE) {
		key++;
		zv = zend_hash_find_ex(EG(zend_constants), Z_STR_P(key), 1);
		if (zv) {
			c = (zend_constant*)Z_PTR_P(zv);
		}
	}

	if (!c) {
		if (!check_defined_only) {
			zend_throw_error(NULL, "Undefined constant '%s'", Z_STRVAL_P(RT_CONSTANT(opline, opline->op2)));
			ZVAL_UNDEF(EX_VAR(opline->result.var));
		}
		CACHE_PTR(opline->extended_value, ENCODE_SPECIAL_CACHE_NUM(zend_hash_num_elements(EG(zend_constants))));
		return FAILURE;
	}

	if (!check_defined_only) {
		ZVAL_COPY_OR_DUP(EX_VAR(opline->result.var), &c->value);
	}

	CACHE_PTR(opline->extended_value, c);
	return SUCCESS;
}

void ZEND_FASTCALL zend_jit_get_constant(const zval *key, uint32_t flags)
{
	_zend_quick_get_constant(key, flags, 0);
}

int ZEND_FASTCALL zend_jit_check_constant(const zval *key)
{
	return _zend_quick_get_constant(key, 0, 1);
}

static zend_always_inline ZEND_OPCODE_HANDLER_RET ZEND_FASTCALL zend_jit_trace_counter_helper(uint32_t cost ZEND_OPCODE_HANDLER_ARGS_DC)
{
	zend_jit_op_array_trace_extension *jit_extension =
		(zend_jit_op_array_trace_extension*)ZEND_FUNC_INFO(&EX(func)->op_array);
	size_t offset = jit_extension->offset;
#ifndef HAVE_GCC_GLOBAL_REGS
	const zend_op *opline = EX(opline);
#endif

	*(ZEND_OP_TRACE_INFO(opline, offset)->counter) -= cost;

	if (UNEXPECTED(*(ZEND_OP_TRACE_INFO(opline, offset)->counter) <= 0)) {
		*(ZEND_OP_TRACE_INFO(opline, offset)->counter) = ZEND_JIT_COUNTER_INIT;
		if (UNEXPECTED(zend_jit_trace_hot_root(execute_data, opline) < 0)) {
#ifndef HAVE_GCC_GLOBAL_REGS
			return -1;
#endif
		}
#ifdef HAVE_GCC_GLOBAL_REGS
		execute_data = EG(current_execute_data);
		opline = EX(opline);
		return;
#else
		return 1;
#endif
	} else {
		zend_vm_opcode_handler_t handler = (zend_vm_opcode_handler_t)ZEND_OP_TRACE_INFO(opline, offset)->orig_handler;
		ZEND_OPCODE_TAIL_CALL(handler);
	}
}

ZEND_OPCODE_HANDLER_RET ZEND_FASTCALL zend_jit_func_trace_helper(ZEND_OPCODE_HANDLER_ARGS)
{
	ZEND_OPCODE_TAIL_CALL_EX(zend_jit_trace_counter_helper,
		((ZEND_JIT_COUNTER_INIT + JIT_G(hot_func) - 1) / JIT_G(hot_func)));
}

ZEND_OPCODE_HANDLER_RET ZEND_FASTCALL zend_jit_ret_trace_helper(ZEND_OPCODE_HANDLER_ARGS)
{
	ZEND_OPCODE_TAIL_CALL_EX(zend_jit_trace_counter_helper,
		((ZEND_JIT_COUNTER_INIT + JIT_G(hot_return) - 1) / JIT_G(hot_return)));
}

ZEND_OPCODE_HANDLER_RET ZEND_FASTCALL zend_jit_loop_trace_helper(ZEND_OPCODE_HANDLER_ARGS)
{
	ZEND_OPCODE_TAIL_CALL_EX(zend_jit_trace_counter_helper,
		((ZEND_JIT_COUNTER_INIT + JIT_G(hot_loop) - 1) / JIT_G(hot_loop)));
}

#define TRACE_RECORD(_op, _info, _ptr) \
	trace_buffer[idx].info = _op | (_info); \
	trace_buffer[idx].ptr = _ptr; \
	idx++; \
	if (idx >= ZEND_JIT_TRACE_MAX_LENGTH - 1) { \
		stop = ZEND_JIT_TRACE_STOP_TOO_LONG; \
		break; \
	}

#define TRACE_RECORD_VM(_op, _ptr, _op1_type, _op2_type, _op3_type) \
	trace_buffer[idx].op = _op; \
	trace_buffer[idx].op1_type = _op1_type; \
	trace_buffer[idx].op2_type = _op2_type; \
	trace_buffer[idx].op3_type = _op3_type; \
	trace_buffer[idx].ptr = _ptr; \
	idx++; \
	if (idx >= ZEND_JIT_TRACE_MAX_LENGTH - 1) { \
		stop = ZEND_JIT_TRACE_STOP_TOO_LONG; \
		break; \
	}

#define TRACE_START(_op, _start, _ptr1, _ptr2) \
	trace_buffer[0].op = _op; \
	trace_buffer[0].start = _start; \
	trace_buffer[0].level = 0; \
	trace_buffer[0].ptr = _ptr1; \
	trace_buffer[1].last = 0; \
	trace_buffer[1].ptr = _ptr2; \
	idx = ZEND_JIT_TRACE_START_REC_SIZE;

#define TRACE_END(_op, _stop, _ptr) \
	trace_buffer[1].last = idx; \
	trace_buffer[idx].op   = _op; \
	trace_buffer[idx].start = trace_buffer[idx].start; \
	trace_buffer[idx].stop = trace_buffer[0].stop = _stop; \
	trace_buffer[idx].level = trace_buffer[0].level = ret_level ? ret_level + 1 : 0; \
	trace_buffer[idx].ptr  = _ptr;

static int zend_jit_trace_recursive_call_count(const zend_op_array *op_array, const zend_op_array **unrolled_calls, int ret_level, int level)
{
	int i;
	int count = 0;

	for (i = ret_level; i < level; i++) {
		count += (unrolled_calls[i] == op_array);
	}
	return count;
}

static int zend_jit_trace_recursive_ret_count(const zend_op_array *op_array, const zend_op_array **unrolled_calls, int ret_level)
{
	int i;
	int count = 0;

	for (i = 0; i < ret_level; i++) {
		count += (unrolled_calls[i] == op_array);
	}
	return count;
}

static int zend_jit_trace_has_recursive_ret(zend_execute_data *ex, const zend_op_array *orig_op_array, const zend_op *orig_opline, int ret_level)
{
	while (ex != NULL && ret_level < ZEND_JIT_TRACE_MAX_RET_DEPTH) {
		if (&ex->func->op_array == orig_op_array && ex->opline + 1 == orig_opline) {
			return 1;
		}
		ex = ex->prev_execute_data;
		ret_level++;
	}
	return 0;
}

static int zend_jit_trace_bad_inner_loop(const zend_op *opline)
{
	const zend_op **cache_opline = JIT_G(bad_root_cache_opline);
	uint8_t *cache_count = JIT_G(bad_root_cache_count);
	uint8_t *cache_stop = JIT_G(bad_root_cache_stop);
	uint32_t i;

	for (i = 0; i < ZEND_JIT_TRACE_BAD_ROOT_SLOTS; i++) {
		if (cache_opline[i] == opline) {
			if ((cache_stop[i] == ZEND_JIT_TRACE_STOP_INNER_LOOP
			  || cache_stop[i] == ZEND_JIT_TRACE_STOP_LOOP_EXIT)
			 && cache_count[i] > JIT_G(blacklist_root_trace) / 2) {
				return 1;
			}
			break;
		}
	}
	return 0;
}

static int zend_jit_trace_bad_compiled_loop(const zend_op *opline)
{
	const zend_op **cache_opline = JIT_G(bad_root_cache_opline);
	uint8_t *cache_count = JIT_G(bad_root_cache_count);
	uint8_t *cache_stop = JIT_G(bad_root_cache_stop);
	uint32_t i;

	for (i = 0; i < ZEND_JIT_TRACE_BAD_ROOT_SLOTS; i++) {
		if (cache_opline[i] == opline) {
			if (cache_stop[i] == ZEND_JIT_TRACE_STOP_COMPILED_LOOP
			 && cache_count[i] >= JIT_G(blacklist_root_trace) - 1) {
				return 1;
			}
			break;
		}
	}
	return 0;
}

static int zend_jit_trace_bad_loop_exit(const zend_op *opline)
{
	const zend_op **cache_opline = JIT_G(bad_root_cache_opline);
	uint8_t *cache_count = JIT_G(bad_root_cache_count);
	uint8_t *cache_stop = JIT_G(bad_root_cache_stop);
	uint32_t i;

	for (i = 0; i < ZEND_JIT_TRACE_BAD_ROOT_SLOTS; i++) {
		if (cache_opline[i] == opline) {
			if (cache_stop[i] == ZEND_JIT_TRACE_STOP_LOOP_EXIT
			 && cache_count[i] >= JIT_G(blacklist_root_trace) - 1) {
				return 1;
			}
			break;
		}
	}
	return 0;
}

static int zend_jit_trace_record_fake_init_call_ex(zend_execute_data *call, zend_jit_trace_rec *trace_buffer, int idx, zend_bool is_megamorphic, uint32_t *megamorphic, uint32_t level, uint32_t *call_level)
{
	zend_jit_trace_stop stop ZEND_ATTRIBUTE_UNUSED = ZEND_JIT_TRACE_STOP_ERROR;

	do {
		zend_function *func;
		zend_jit_op_array_trace_extension *jit_extension;

		if (call->prev_execute_data) {
			idx = zend_jit_trace_record_fake_init_call_ex(call->prev_execute_data, trace_buffer, idx, is_megamorphic, megamorphic, level, call_level);
			if (idx < 0) {
				return idx;
			}
		}

		func = call->func;
		if (func->common.fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE) {
			/* TODO: Can we continue recording ??? */
			return -1;
		}
		if (func->type == ZEND_USER_FUNCTION
		 && (func->op_array.fn_flags & ZEND_ACC_CLOSURE)) {
			jit_extension =
				(zend_jit_op_array_trace_extension*)ZEND_FUNC_INFO(&func->op_array);
			if (UNEXPECTED(!jit_extension || (func->op_array.fn_flags & ZEND_ACC_FAKE_CLOSURE))) {
				return -1;
			}
			func = (zend_function*)jit_extension->op_array;
		}
		if (is_megamorphic
		 /* TODO: use more accurate check ??? */
		 && ((ZEND_CALL_INFO(call) & ZEND_CALL_DYNAMIC)
		  || func->common.scope)) {
			func = NULL;
			*megamorphic |= (1 << (level + *call_level));
		} else {
			*megamorphic &= ~(1 << (level + *call_level));
		}
		(*call_level)++;
		TRACE_RECORD(ZEND_JIT_TRACE_INIT_CALL, ZEND_JIT_TRACE_FAKE_INIT_CALL, func);
	} while (0);
	return idx;
}

static int zend_jit_trace_record_fake_init_call(zend_execute_data *call, zend_jit_trace_rec *trace_buffer, int idx, zend_bool is_megamorphic, uint32_t *megamorphic, uint32_t level)
{
	uint32_t call_level = 0;

	return zend_jit_trace_record_fake_init_call_ex(call, trace_buffer, idx, is_megamorphic, megamorphic, level, &call_level);
}

static int zend_jit_trace_call_level(const zend_execute_data *call)
{
	int call_level = 0;

	while (call->prev_execute_data) {
		call_level++;
		call = call->prev_execute_data;
	}
	return call_level;
}

/*
 *  Trace Linking Rules
 *  ===================
 *
 *                                          flags
 *          +----------+----------+----------++----------+----------+----------+
 *          |                                ||              JIT               |
 *          +----------+----------+----------++----------+----------+----------+
 *   start  |   LOOP   |  ENTER   |  RETURN  ||   LOOP   |  ENTER   |  RETURN  |
 * +========+==========+==========+==========++==========+==========+==========+
 * | LOOP   |   loop   |          | loop-ret || COMPILED |   LINK   |   LINK   |
 * +--------+----------+----------+----------++----------+----------+----------+
 * | ENTER  |INNER_LOOP| rec-call |  return  ||   LINK   |   LINK   |   LINK   |
 * +--------+----------+----------+----------++----------+----------+----------+
 * | RETURN |INNER_LOOP|          |  rec-ret ||   LINK   |          |   LINK   |
 * +--------+----------+----------+----------++----------+----------+----------+
 * | SIDE   |  unroll  |          |  return  ||   LINK   |   LINK   |   LINK   |
 * +--------+----------+----------+----------++----------+----------+----------+
 *
 * loop:       LOOP if "cycle" and level == 0, otherwise INNER_LOOP
 * INNER_LOOP: abort recording and start new one (wait for loop)
 * COMPILED:   abort recording (wait while side exit creates outer loop)
 * unroll:     continue recording while unroll limit reached
 * rec-call:   RECURSIVE_CALL if "cycle" and level > N, otherwise continue
 * loop-ret:   LOOP_EXIT if level == 0, otherwise continue (wait for loop)
 * return:     RETURN if level == 0
 * rec_ret:    RECURSIVE_RET if "cycle" and ret_level > N, otherwise continue
 *
 */

zend_jit_trace_stop ZEND_FASTCALL zend_jit_trace_execute(zend_execute_data *ex, const zend_op *op, zend_jit_trace_rec *trace_buffer, uint8_t start, zend_bool is_megamorphic)

{
#ifdef HAVE_GCC_GLOBAL_REGS
	zend_execute_data *save_execute_data = execute_data;
	const zend_op *save_opline = opline;
#endif
	const zend_op *orig_opline, *end_opline;
	zend_jit_trace_stop stop = ZEND_JIT_TRACE_STOP_ERROR;
	int level = 0;
	int ret_level = 0;
	int call_level;
	zend_vm_opcode_handler_t handler;
	const zend_op_array *op_array;
	zend_jit_op_array_trace_extension *jit_extension;
	size_t offset;
	int idx, count;
	uint8_t  trace_flags, op1_type, op2_type, op3_type;
	zend_class_entry *ce1, *ce2;
	int backtrack_recursion = -1;
	int backtrack_ret_recursion = -1;
	int backtrack_ret_recursion_level = 0;
	int loop_unroll_limit = 0;
	uint32_t megamorphic = 0;
	const zend_op_array *unrolled_calls[ZEND_JIT_TRACE_MAX_CALL_DEPTH + ZEND_JIT_TRACE_MAX_RET_DEPTH];
#ifdef HAVE_GCC_GLOBAL_REGS
	zend_execute_data *prev_execute_data = ex;

	execute_data = ex;
	opline = EX(opline) = op;
#else
	int rc;
	zend_execute_data *execute_data = ex;
	const zend_op *opline = EX(opline);
#endif
	zend_execute_data *prev_call = EX(call);

	if (UNEXPECTED(opline->opcode == ZEND_HANDLE_EXCEPTION)) {
		/* Abort trace because of exception */
		return ZEND_JIT_TRACE_STOP_EXCEPTION;
	}

	orig_opline = opline;

	op_array = &EX(func)->op_array;
	jit_extension =
		(zend_jit_op_array_trace_extension*)ZEND_FUNC_INFO(op_array);
	offset = jit_extension->offset;
	if (!op_array->function_name
	 || (op_array->fn_flags & ZEND_ACC_CLOSURE)) {
		op_array = jit_extension->op_array;
	}

	TRACE_START(ZEND_JIT_TRACE_START, start, op_array, opline);

	if (prev_call) {
		int ret = zend_jit_trace_record_fake_init_call(prev_call, trace_buffer, idx, is_megamorphic, &megamorphic, ret_level + level);
		if (ret < 0) {
			return ZEND_JIT_TRACE_STOP_BAD_FUNC;
		}
		idx = ret;
	}

	while (1) {
		ce1 = ce2 = NULL;
		op1_type = op2_type = op3_type = IS_UNKNOWN;
		if ((opline->op1_type & (IS_TMP_VAR|IS_VAR|IS_CV))
		 && opline->opcode != ZEND_ROPE_ADD
		 && opline->opcode != ZEND_ROPE_END
		 && opline->opcode != ZEND_NEW
		 && opline->opcode != ZEND_FETCH_CLASS_CONSTANT
		 && opline->opcode != ZEND_INIT_STATIC_METHOD_CALL) {
			zval *zv = EX_VAR(opline->op1.var);
			op1_type = Z_TYPE_P(zv);
			uint8_t flags = 0;

			if (op1_type == IS_INDIRECT) {
				zv = Z_INDIRECT_P(zv);
				op1_type = Z_TYPE_P(zv);
				flags |= IS_TRACE_INDIRECT;
			}
			if (op1_type == IS_REFERENCE) {
				zv = Z_REFVAL_P(zv);
				op1_type = Z_TYPE_P(zv);
				flags |= IS_TRACE_REFERENCE;
			}
			op1_type |= flags;
			if (Z_TYPE_P(zv) == IS_OBJECT) {
				ce1 = Z_OBJCE_P(zv);
			}
		}
		if (opline->op2_type & (IS_TMP_VAR|IS_VAR|IS_CV)
		 && opline->opcode != ZEND_INSTANCEOF
		 && opline->opcode != ZEND_UNSET_STATIC_PROP) {
			zval *zv = EX_VAR(opline->op2.var);
			uint8_t flags = 0;

			op2_type = Z_TYPE_P(zv);
			if (op2_type == IS_INDIRECT) {
				zv = Z_INDIRECT_P(zv);
				op2_type = Z_TYPE_P(zv);
				flags |= IS_TRACE_INDIRECT;
			}
			if (op2_type == IS_REFERENCE) {
				zv = Z_REFVAL_P(zv);
				op2_type = Z_TYPE_P(zv);
				flags |= IS_TRACE_REFERENCE;
			}
			op2_type |= flags;
			if (Z_TYPE_P(zv) == IS_OBJECT) {
				ce2 = Z_OBJCE_P(zv);
			}
		}
		if (opline->opcode == ZEND_ASSIGN_DIM ||
			opline->opcode == ZEND_ASSIGN_OBJ ||
			opline->opcode == ZEND_ASSIGN_STATIC_PROP ||
			opline->opcode == ZEND_ASSIGN_DIM_OP ||
			opline->opcode == ZEND_ASSIGN_OBJ_OP ||
			opline->opcode == ZEND_ASSIGN_STATIC_PROP_OP ||
			opline->opcode == ZEND_ASSIGN_OBJ_REF ||
			opline->opcode == ZEND_ASSIGN_STATIC_PROP_REF) {
			if ((opline+1)->op1_type & (IS_TMP_VAR|IS_VAR|IS_CV)) {
				zval *zv = EX_VAR((opline+1)->op1.var);
				uint8_t flags = 0;

				op3_type = Z_TYPE_P(zv);
				if (op3_type == IS_INDIRECT) {
					zv = Z_INDIRECT_P(zv);
					op3_type = Z_TYPE_P(zv);
					flags |= IS_TRACE_INDIRECT;
				}
				if (op3_type == IS_REFERENCE) {
					zv = Z_REFVAL_P(zv);
					op3_type = Z_TYPE_P(zv);
					flags |= IS_TRACE_REFERENCE;
				}
				op3_type |= flags;
			}
		}

		TRACE_RECORD_VM(ZEND_JIT_TRACE_VM, opline, op1_type, op2_type, op3_type);

		if (ce1) {
			TRACE_RECORD(ZEND_JIT_TRACE_OP1_TYPE, 0, ce1);
		}

		if (ce2) {
			TRACE_RECORD(ZEND_JIT_TRACE_OP2_TYPE, 0, ce2);
		}

		if (opline->opcode == ZEND_DO_FCALL
		 || opline->opcode == ZEND_DO_ICALL
		 || opline->opcode == ZEND_DO_UCALL
		 ||	opline->opcode == ZEND_DO_FCALL_BY_NAME) {
			call_level = zend_jit_trace_call_level(EX(call));
			if (megamorphic & (1 << (ret_level + level + call_level))) {
				stop = ZEND_JIT_TRACE_STOP_INTERPRETER;
				break;
			}
			if (EX(call)->func->type == ZEND_INTERNAL_FUNCTION) {
				TRACE_RECORD(ZEND_JIT_TRACE_DO_ICALL, 0, EX(call)->func);
			}
		} else if (opline->opcode == ZEND_INCLUDE_OR_EVAL) {
			stop = ZEND_JIT_TRACE_STOP_INTERPRETER;
			break;
		}

		handler = (zend_vm_opcode_handler_t)ZEND_OP_TRACE_INFO(opline, offset)->call_handler;
#ifdef HAVE_GCC_GLOBAL_REGS
		handler();
		if (UNEXPECTED(opline == zend_jit_halt_op)) {
			stop = ZEND_JIT_TRACE_STOP_RETURN_HALT;
			break;
		}
		if (UNEXPECTED(execute_data != prev_execute_data)) {
#else
		rc = handler(ZEND_OPCODE_HANDLER_ARGS_PASSTHRU);
		if (rc != 0) {
			if (rc < 0) {
				stop = ZEND_JIT_TRACE_STOP_RETURN_HALT;
				break;
			}
			execute_data = EG(current_execute_data);
			opline = EX(opline);
#endif

            op_array = &EX(func)->op_array;
			jit_extension =
				(zend_jit_op_array_trace_extension*)ZEND_FUNC_INFO(op_array);
			if (UNEXPECTED(!jit_extension)) {
				stop = ZEND_JIT_TRACE_STOP_BAD_FUNC;
				break;
			}
			offset = jit_extension->offset;
			if (!op_array->function_name
			 || (op_array->fn_flags & ZEND_ACC_CLOSURE)) {
				op_array = jit_extension->op_array;
			}

#ifdef HAVE_GCC_GLOBAL_REGS
			if (execute_data->prev_execute_data == prev_execute_data) {
#else
			if (rc == 1) {
#endif
				/* Enter into function */
				prev_call = NULL;
				if (level > ZEND_JIT_TRACE_MAX_CALL_DEPTH) {
					stop = ZEND_JIT_TRACE_STOP_TOO_DEEP;
					break;
				}

				if (EX(func)->op_array.fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE) {
					/* TODO: Can we continue recording ??? */
					stop = ZEND_JIT_TRACE_STOP_TRAMPOLINE;
					break;
				}

				TRACE_RECORD(ZEND_JIT_TRACE_ENTER,
					EX(return_value) != NULL ? ZEND_JIT_TRACE_RETRUN_VALUE_USED : 0,
					op_array);

				count = zend_jit_trace_recursive_call_count(&EX(func)->op_array, unrolled_calls, ret_level, level);

				if (opline == orig_opline) {
					if (count + 1 >= JIT_G(max_recursive_calls)) {
						stop = ZEND_JIT_TRACE_STOP_RECURSIVE_CALL;
						break;
					}
					backtrack_recursion = idx;
				} else if (count >= JIT_G(max_recursive_calls)) {
					stop = ZEND_JIT_TRACE_STOP_DEEP_RECURSION;
					break;
				}

				unrolled_calls[ret_level + level] = &EX(func)->op_array;
				level++;
			} else {
				/* Return from function */
				prev_call = EX(call);
				if (level == 0) {
					if (start == ZEND_JIT_TRACE_START_RETURN
					        && JIT_G(max_recursive_returns) > 0
					        && execute_data->prev_execute_data
					        && execute_data->prev_execute_data->func
					        && execute_data->prev_execute_data->func->type == ZEND_USER_FUNCTION
					        && zend_jit_trace_has_recursive_ret(execute_data, trace_buffer[0].op_array, orig_opline, ret_level)) {
						if (ret_level > ZEND_JIT_TRACE_MAX_RET_DEPTH) {
							stop = ZEND_JIT_TRACE_STOP_TOO_DEEP_RET;
							break;
						}
						TRACE_RECORD(ZEND_JIT_TRACE_BACK, 0, op_array);
						count = zend_jit_trace_recursive_ret_count(&EX(func)->op_array, unrolled_calls, ret_level);
						if (opline == orig_opline) {
							if (count + 1 >= JIT_G(max_recursive_returns)) {
								stop = ZEND_JIT_TRACE_STOP_RECURSIVE_RET;
								break;
							}
							backtrack_ret_recursion = idx;
							backtrack_ret_recursion_level = ret_level;
						} else if (count >= JIT_G(max_recursive_returns)) {
							stop = ZEND_JIT_TRACE_STOP_DEEP_RECURSION;
							break;
						}

						unrolled_calls[ret_level] = &EX(func)->op_array;
						ret_level++;

						if (prev_call) {
							int ret = zend_jit_trace_record_fake_init_call(prev_call, trace_buffer, idx, 0, &megamorphic, ret_level + level);
							if (ret < 0) {
								stop = ZEND_JIT_TRACE_STOP_BAD_FUNC;
								break;
							}
							idx = ret;
						}
					} else if (start & ZEND_JIT_TRACE_START_LOOP
					 && !zend_jit_trace_bad_loop_exit(orig_opline)) {
						/* Fail to try close the loop.
						   If this doesn't work terminate it. */
						stop = ZEND_JIT_TRACE_STOP_LOOP_EXIT;
						break;
					} else {
						stop = ZEND_JIT_TRACE_STOP_RETURN;
						break;
					}
				} else {
					level--;
					TRACE_RECORD(ZEND_JIT_TRACE_BACK, 0, op_array);
				}
			}
#ifdef HAVE_GCC_GLOBAL_REGS
			prev_execute_data = execute_data;
#endif
		}
		if (EX(call) != prev_call) {
			if (EX(call)
			 && EX(call)->prev_execute_data == prev_call) {
				zend_function *func;
				zend_jit_op_array_trace_extension *jit_extension;

				if (EX(call)->func->common.fn_flags & ZEND_ACC_CALL_VIA_TRAMPOLINE) {
					/* TODO: Can we continue recording ??? */
					stop = ZEND_JIT_TRACE_STOP_TRAMPOLINE;
					break;
				}
				func = EX(call)->func;
				if (func->type == ZEND_USER_FUNCTION
				 && (func->op_array.fn_flags & ZEND_ACC_CLOSURE)) {
					jit_extension =
						(zend_jit_op_array_trace_extension*)ZEND_FUNC_INFO(&func->op_array);
					if (UNEXPECTED(!jit_extension) || (func->op_array.fn_flags & ZEND_ACC_FAKE_CLOSURE)) {
						stop = ZEND_JIT_TRACE_STOP_INTERPRETER;
						break;
					}
					func = (zend_function*)jit_extension->op_array;
				}

#ifndef HAVE_GCC_GLOBAL_REGS
				opline = EX(opline);
#endif

				if (JIT_G(max_polymorphic_calls) == 0
				 && zend_jit_may_be_polymorphic_call(opline - 1)) {
					func = NULL;
				}
				call_level = zend_jit_trace_call_level(EX(call));
				ZEND_ASSERT(ret_level + level + call_level < 32);
				if (func) {
					megamorphic &= ~(1 << (ret_level + level + call_level));
				} else {
					megamorphic |= (1 << (ret_level + level + call_level));
				}
				TRACE_RECORD(ZEND_JIT_TRACE_INIT_CALL, 0, func);
			}
			prev_call = EX(call);
		}

#ifndef HAVE_GCC_GLOBAL_REGS
		opline = EX(opline);
#endif

		if (UNEXPECTED(opline->opcode == ZEND_HANDLE_EXCEPTION)) {
			/* Abort trace because of exception */
			stop = ZEND_JIT_TRACE_STOP_EXCEPTION;
			break;
		}

		trace_flags = ZEND_OP_TRACE_INFO(opline, offset)->trace_flags;
		if (trace_flags) {
			if (trace_flags & ZEND_JIT_TRACE_JITED) {
				if (trace_flags & ZEND_JIT_TRACE_START_LOOP) {
					if ((start & ZEND_JIT_TRACE_START_LOOP) != 0
					 && level + ret_level == 0
					 && !zend_jit_trace_bad_compiled_loop(orig_opline)) {
						/* Fail to try close outer loop through side exit.
						   If this doesn't work just link. */
						stop = ZEND_JIT_TRACE_STOP_COMPILED_LOOP;
						break;
					} else {
						stop = ZEND_JIT_TRACE_STOP_LINK;
						break;
					}
				} else if (trace_flags & ZEND_JIT_TRACE_START_ENTER) {
					if (start != ZEND_JIT_TRACE_START_RETURN) {
						// TODO: We may try to inline function ???
						stop = ZEND_JIT_TRACE_STOP_LINK;
						break;
					}
				} else {
					stop = ZEND_JIT_TRACE_STOP_LINK;
					break;
				}
			} else if (trace_flags & ZEND_JIT_TRACE_BLACKLISTED) {
				stop = ZEND_JIT_TRACE_STOP_BLACK_LIST;
				break;
			} else if (trace_flags & ZEND_JIT_TRACE_START_LOOP) {
				if (start != ZEND_JIT_TRACE_START_SIDE) {
					if (opline == orig_opline && level + ret_level == 0) {
						stop = ZEND_JIT_TRACE_STOP_LOOP;
						break;
					}
					/* Fail to try creating a trace for inner loop first.
					   If this doesn't work try unroling loop. */
					if (!zend_jit_trace_bad_inner_loop(opline)) {
						stop = ZEND_JIT_TRACE_STOP_INNER_LOOP;
						break;
					}
				}
				if (loop_unroll_limit < JIT_G(max_loops_unroll)) {
					loop_unroll_limit++;
				} else {
					stop = ZEND_JIT_TRACE_STOP_LOOP_UNROLL;
					break;
				}
			} else if (trace_flags & ZEND_JIT_TRACE_UNSUPPORTED) {
				TRACE_RECORD(ZEND_JIT_TRACE_VM, 0, opline);
				stop = ZEND_JIT_TRACE_STOP_NOT_SUPPORTED;
				break;
			}
		}
	}

	end_opline = opline;
	if (!ZEND_JIT_TRACE_STOP_OK(stop)) {
		if (backtrack_recursion > 0) {
			idx = backtrack_recursion;
			stop = ZEND_JIT_TRACE_STOP_RECURSIVE_CALL;
			end_opline = orig_opline;
		} else if (backtrack_ret_recursion > 0) {
			idx = backtrack_ret_recursion;
			ret_level = backtrack_ret_recursion_level;
			stop = ZEND_JIT_TRACE_STOP_RECURSIVE_RET;
			end_opline = orig_opline;
		}
	}

	if (stop == ZEND_JIT_TRACE_STOP_LINK) {
		/* Shrink fake INIT_CALLs */
		while (trace_buffer[idx-1].op == ZEND_JIT_TRACE_INIT_CALL
				&& (trace_buffer[idx-1].info & ZEND_JIT_TRACE_FAKE_INIT_CALL)) {
			idx--;
		}
	}

	TRACE_END(ZEND_JIT_TRACE_END, stop, end_opline);

#ifdef HAVE_GCC_GLOBAL_REGS
	if (stop != ZEND_JIT_TRACE_STOP_HALT
	 && stop != ZEND_JIT_TRACE_STOP_RETURN_HALT) {
		EX(opline) = opline;
	}
#endif

#ifdef HAVE_GCC_GLOBAL_REGS
	execute_data = save_execute_data;
	opline = save_opline;
#endif

	return stop;
}
