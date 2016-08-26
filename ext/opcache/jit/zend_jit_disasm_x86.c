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
   |          Xinchen Hui <xinchen.h@zend.com>                            |
   +----------------------------------------------------------------------+
*/

#define HAVE_DISASM 1
#define DISASM_INTEL_SYNTAX 0

#include "jit/libudis86/itab.c"
#include "jit/libudis86/decode.c"
#include "jit/libudis86/syn.c"
#if DISASM_INTEL_SYNTAX
# include "jit/libudis86/syn-intel.c"
#else
# include "jit/libudis86/syn-att.c"
#endif
#include "jit/libudis86/udis86.c"

static void zend_jit_disasm_add_symbol(const char *name,
                                       uint64_t    addr,
                                       uint64_t    size);

#include "jit/zend_elf.c"

#include "zend_sort.h"

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <dlfcn.h>

static struct ud ud;

typedef struct _sym_node {
	uint64_t          addr;
	uint64_t          size;
	struct _sym_node *parent;
	struct _sym_node *child[2];
	unsigned char     info;
	char              name[1];
} zend_sym_node;

static zend_sym_node *symbols = NULL;

static void zend_syms_rotateleft(zend_sym_node *p) {
	zend_sym_node *r = p->child[1];
	p->child[1] = r->child[0];
	if (r->child[0]) {
		r->child[0]->parent = p;
	}
	r->parent = p->parent;
	if (p->parent == NULL) {
		symbols = r;
	} else if (p->parent->child[0] == p) {
		p->parent->child[0] = r;
	} else {
		p->parent->child[1] = r;
	}
	r->child[0] = p;
	p->parent = r;
}

static void zend_syms_rotateright(zend_sym_node *p) {
	zend_sym_node *l = p->child[0];
	p->child[0] = l->child[1];
	if (l->child[1]) {
		l->child[1]->parent = p;
	}
	l->parent = p->parent;
	if (p->parent == NULL) {
		symbols = l;
	} else if (p->parent->child[1] == p) {
		p->parent->child[1] = l;
	} else {
		p->parent->child[0] = l;
	}
	l->child[1] = p;
	p->parent = l;
}

static void zend_jit_disasm_add_symbol(const char *name,
                                       uint64_t    addr,
                                       uint64_t    size)
{
	zend_sym_node *sym;
	size_t len = strlen(name);

	sym = malloc(sizeof(zend_sym_node) + len + 1);
	if (!sym) {
		return;
	}
	sym->addr = addr;
	sym->size = size;
	memcpy((char*)&sym->name, name, len + 1);
	sym->parent = sym->child[0] = sym->child[1] = NULL;
	sym->info = 1;
	if (symbols) {
		zend_sym_node *node = symbols;

		/* insert it into rbtree */
		do {
			if (sym->addr > node->addr) {
				if (node->child[1]) {
					node = node->child[1];
				} else {
					node->child[1] = sym;
					sym->parent = node;
					break;
				}
			} else if (sym->addr < node->addr) {
				if (node->child[0]) {
					node = node->child[0];
				} else {
					node->child[0] = sym;
					sym->parent = node;
					break;
				}
			} else {
				ZEND_ASSERT(sym->addr == node->addr);
				free(sym);
				return;
			}
		} while (1);

		/* fix rbtree after instering */
		while (sym && sym != symbols && sym->parent->info == 1) {
			if (sym->parent == sym->parent->parent->child[0]) {
				node = sym->parent->parent->child[1];
				if (node && node->info == 1) {
					sym->parent->info = 0;
					node->info = 0;
					sym->parent->parent->info = 1;
					sym = sym->parent->parent;
				} else {
					if (sym == sym->parent->child[1]) {
						sym = sym->parent;
						zend_syms_rotateleft(sym);
					}
					sym->parent->info = 0;
					sym->parent->parent->info = 1;
					zend_syms_rotateright(sym->parent->parent);
				}
			} else {
				node = sym->parent->parent->child[0];
				if (node && node->info == 1) {
					sym->parent->info = 0;
					node->info = 0;
					sym->parent->parent->info = 1;
					sym = sym->parent->parent;
				} else {
					if (sym == sym->parent->child[0]) {
						sym = sym->parent;
						zend_syms_rotateright(sym);
					}
					sym->parent->info = 0;
					sym->parent->parent->info = 1;
					zend_syms_rotateleft(sym->parent->parent);
				}
			}
		}
	} else {
		symbols = sym;
	}
	symbols->info = 0;
}

static void zend_jit_disasm_destroy_symbols(zend_sym_node *n) {
	if (n) {
		if (n->child[0]) {
			zend_jit_disasm_destroy_symbols(n->child[0]);
		} else if (n->child[1]) {
			zend_jit_disasm_destroy_symbols(n->child[1]);
		}
		free(n);
	}
}

static const char* zend_jit_disasm_find_symbol(uint64_t  addr,
                                               int64_t  *offset) {
	zend_sym_node *node = symbols;
	while (node) {
		if (addr < node->addr) {
			node = node->child[0];
		} else if (addr > (node->addr + node->size)) {
			node = node->child[1];
		} else {
			*offset = addr - node->addr;
			return node->name;
		}
	}
	return NULL;
}

static const char* zend_jit_disasm_resolver(struct ud *ud,
                                            uint64_t   addr,
                                            int64_t   *offset)
{
	((void)ud);
	const char *name;
	void *a = (void*)(zend_uintptr_t)(addr);
	Dl_info info;

	name = zend_jit_disasm_find_symbol(addr, offset);
	if (name) {
		return name;
	}

	if (dladdr(a, &info)
	 && info.dli_sname != NULL
	 && info.dli_saddr == a) {
		return info.dli_sname;
	}

	return NULL;
}

static int zend_jit_cmp_labels(Bucket *b1, Bucket *b2)
{
	return ((b1->h > b2->h) > 0) ? 1 : -1;
}

static int zend_jit_disasm(const char *name,
                           const char *filename,
                           const void *start,
                           size_t      size)
{
	const void *end = (void *)((char *)start + size);
	zval zv, *z;
	zend_long n;
	HashTable labels;
	const struct ud_operand *op;
	uint64_t addr;

	if (name) {
		fprintf(stderr, "%s: ; (%s)\n", name, filename ? filename : "unknown");
	}

	ud_set_input_buffer(&ud, (uint8_t*)start, (uint8_t*)end - (uint8_t*)start);
	ud_set_pc(&ud, (uint64_t)(uintptr_t)start);

	zend_hash_init(&labels, 8, NULL, NULL, 0);
	ZVAL_NULL(&zv);
	while (ud_disassemble(&ud)) {
		op = ud_insn_opr(&ud, 0);
		if (op && op->type == UD_OP_JIMM) {
			addr = ud_syn_rel_target(&ud, (struct ud_operand*)op);
			if (addr >= (uint64_t)(uintptr_t)start && addr < (uint64_t)(uintptr_t)end) {
				zend_hash_index_add(&labels, addr, &zv);
			}
		}
	}

	zend_hash_sort(&labels, (compare_func_t)zend_jit_cmp_labels, 0);

	/* label numbering */
	n = 0;
	ZEND_HASH_FOREACH_VAL(&labels, z) {
		n++;
		ZVAL_LONG(z, n);
	} ZEND_HASH_FOREACH_END();

	ud_set_input_buffer(&ud, (uint8_t*)start, (uint8_t*)end - (uint8_t*)start);
	ud_set_pc(&ud, (uint64_t)(uintptr_t)start);

	while (ud_disassemble(&ud)) {
		addr = ud_insn_off(&ud);
		z = zend_hash_index_find(&labels, addr);
		if (z) {
			fprintf(stderr, ".L" ZEND_LONG_FMT ":\n", Z_LVAL_P(z));
		}
		op = ud_insn_opr(&ud, 0);
		if (op && op->type == UD_OP_JIMM) {
			addr = ud_syn_rel_target(&ud, (struct ud_operand*)op);
			if (addr >= (uint64_t)(uintptr_t)start && addr < (uint64_t)(uintptr_t)end) {
				z = zend_hash_index_find(&labels, addr);
				if (z) {
					const char *str = ud_insn_asm(&ud);
					int len;

					len = 0;
					while (str[len] != 0 && str[len] != ' ' && str[len] != '\t') {
						len++;
					}
					if (str[len] != 0) {
						while (str[len] == ' ' || str[len] == '\t') {
							len++;
						}
						fprintf(stderr, "\t%.*s.L" ZEND_LONG_FMT "\n", len, str, Z_LVAL_P(z));
						continue;
					}
				}
			}
		}
		fprintf(stderr, "\t%s\n", ud_insn_asm(&ud));
	}
	fprintf(stderr, "\n");

	zend_hash_destroy(&labels);

	return 1;
}

static int zend_jit_disasm_init(void)
{
	ud_init(&ud);
#ifdef __x86_64__
	ud_set_mode(&ud, 64);
#else
	ud_set_mode(&ud, 32);
#endif
#if DISASM_INTEL_SYNTAX
	ud_set_syntax(&ud, UD_SYN_INTEL);
#else
	ud_set_syntax(&ud, UD_SYN_ATT);
#endif
	ud_set_sym_resolver(&ud, zend_jit_disasm_resolver);
#ifndef ZTS
	zend_jit_disasm_add_symbol("executor_globals", (uint64_t)(uintptr_t)&executor_globals, sizeof(executor_globals));
	zend_jit_disasm_add_symbol("compiler_globals", (uint64_t)(uintptr_t)&compiler_globals, sizeof(compiler_globals));
#endif
	zend_elf_load_symbols();

	return 1;
}

static void zend_jit_disasm_shutdown(void)
{
	zend_jit_disasm_destroy_symbols(symbols);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
