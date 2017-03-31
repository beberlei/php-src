/*
   +----------------------------------------------------------------------+
   | Zend JIT                                                             |
   +----------------------------------------------------------------------+
   | Copyright (c) 1998-2016 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Dmitry Stogov <dmitry@zend.com>                             |
   +----------------------------------------------------------------------+
*/

/* $Id:$ */

#ifndef HAVE_JIT_H
#define HAVE_JIT_H

#define ZEND_JIT_LEVEL_NONE        0     /* no JIT */
#define ZEND_JIT_LEVEL_MINIMAL     1     /* minimal JIT (subroutine threading) */
#define ZEND_JIT_LEVEL_INLINE      2     /* selective inline threading */
#define ZEND_JIT_LEVEL_OPT_FUNC    3     /* optimized JIT based on Type-Inference */
#define ZEND_JIT_LEVEL_OPT_FUNCS   4     /* optimized JIT based on Type-Inference and call-tree */
#define ZEND_JIT_LEVEL_OPT_SCRIPT  5     /* optimized JIT based on Type-Inference and inner-procedure analises */

#define ZEND_JIT_LEVEL(n)          ((n) % 10)

#define ZEND_JIT_ON_SCRIPT_LOAD    0
#define ZEND_JIT_ON_FIRST_EXEC     1
#define ZEND_JIT_ON_PROF_REQUEST   2     /* compile the most frequently caled on first requrest functions */
#define ZEND_JIT_ON_HOT_COUNTERS   3     /* compile functions after N calls or loop iterations */
#define ZEND_JIT_ON_DOC_COMMENT    4     /* compile functions with "@jit" tag in doc-comments */

#define ZEND_JIT_TRIGGER(n)        (((n) / 10) % 10)

#define ZEND_JIT_REG_ALLOC_NONE    0     /* no register allocation          */
#define ZEND_JIT_REG_ALLOC_ENABLED 1     /* linear scan register allocation */

#define ZEND_JIT_REG_ALLOC(n)      (((n) / 100) % 10)

#define ZEND_JIT_DEFAULT           "5"


/* Makes profile based JIT (opcache.jit=2*) to generate code only for most
 * offten called functions (above the threshold).
 * TODO: this setting should be configurable
 */
#define ZEND_JIT_PROF_THRESHOLD    0.005

/* Hot Counters based JIT parameters.
 * TODO: this setting should be configurable
 */
#define ZEND_JIT_HOT_FUNC_COST     1
#define ZEND_JIT_HOT_LOOP_COST     2
#define ZEND_JIT_HOT_COUNTER_INIT  127

#define ZEND_JIT_DEBUG_ASM       (1<<0)
#define ZEND_JIT_DEBUG_SSA       (1<<1)
#define ZEND_JIT_DEBUG_REG_ALLOC (1<<2)

#define ZEND_JIT_DEBUG_GDB       (1<<4)
#define ZEND_JIT_DEBUG_PERF      (1<<5)
#define ZEND_JIT_DEBUG_OPROFILE  (1<<6)
#define ZEND_JIT_DEBUG_VTUNE     (1<<7)
#define ZEND_JIT_DEBUG_PERF_DUMP (1<<8)

ZEND_API int  zend_jit_op_array(zend_op_array *op_array, zend_script *script);
ZEND_API int  zend_jit_script(zend_script *script);
ZEND_API void zend_jit_unprotect(void);
ZEND_API void zend_jit_protect(void);
ZEND_API int  zend_jit_startup(zend_long jit, size_t size);
ZEND_API void zend_jit_shutdown(void);
ZEND_API void zend_jit_activate(void);
ZEND_API void zend_jit_deactivate(void);
ZEND_API void zend_jit_status(zval *ret);

typedef struct _zend_lifetime_interval zend_lifetime_interval;
typedef struct _zend_life_range zend_life_range;

struct _zend_life_range {
	uint32_t         start;
	uint32_t         end;
	zend_life_range *next;
};

struct _zend_lifetime_interval {
	int                     ssa_var;
	int8_t                  reg;
	zend_bool               split;
	zend_life_range         range;
	zend_lifetime_interval *list_next;
};

#endif /* HAVE_JIT_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
