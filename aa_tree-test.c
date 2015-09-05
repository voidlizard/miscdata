#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "aa_tree.h"
#include "prng.h"

static void *__alloc(void *cc, size_t n) {
    return malloc(n);
}

static void __dealloc(void *cc, void *c) {
    free(c);
}

static bool __u32_less(void *a, void *b) {
    return *(uint32_t*)a < *(uint32_t*)b;
}

static bool __u32_eq(void *a, void *b) {
    return *(uint32_t*)a == *(uint32_t*)b;
}

static void __u32_cpy(void *a, void *b) {
    *(uint32_t*)a = *(uint32_t*)b;
}

static void __print_u32_dot( void *cc
                           , struct aa_node_info *p
                           , struct aa_node_info *n) {

    char tmp[128];

    if( !p ) {
        snprintf(tmp, sizeof(tmp), "(root)");
    } else {
        snprintf(tmp, sizeof(tmp), "\"(%u,%zu)\"", *(uint32_t*)p->value, p->level);
    }

    if( n ) {
        fprintf( stdout
               , "%s -> " "\"(%u,%zu)\";\n"
               , tmp
               , *(uint32_t*)n->value, n->level);
    }

}

static void __print_u32_digraph( struct aa_tree *t, const char *name ) {
    fprintf(stdout, "digraph %s {\n", name);
    aa_tree_enum_debug(t, 0, __print_u32_dot);
    fprintf(stdout, "}\n");
}

static size_t __less_num = 0;
static size_t __eq_num = 0;

static bool __u32_less_stat(void *a, void *b) {
    __less_num++;
    return *(uint32_t*)a < *(uint32_t*)b;
}

static bool __u32_eq_stat(void *a, void *b) {
    __eq_num++;
    return *(uint32_t*)a == *(uint32_t*)b;
}

void test_aa_tree_create_1(void) {

    char mem[aa_tree_size];

    struct aa_tree *t = aa_tree_create( sizeof(mem)
                                      , mem
                                      , sizeof(uint32_t)
                                      , __u32_less
                                      , __u32_eq
                                      , __u32_cpy
                                      , 0
                                      , __alloc
                                      , __dealloc
                                      );

    uint32_t v[] = { 10, 5, 12, 1, 7, 11, 15 };
    size_t i = 0;

    for(; i < sizeof(v)/sizeof(v[0]); i++ ) {
        aa_tree_insert(t, &v[i]);
    }

    __print_u32_digraph(t, "G");

    aa_tree_destroy(t);
}

void test_aa_tree_remove_1_0(void) {

    char mem[aa_tree_size];

    struct aa_tree *t = aa_tree_create( sizeof(mem)
                                      , mem
                                      , sizeof(uint32_t)
                                      , __u32_less
                                      , __u32_eq
                                      , __u32_cpy
                                      , 0
                                      , __alloc
                                      , __dealloc
                                      );

    uint32_t v[] = { 50 };
    size_t i = 0;

    for(; i < sizeof(v)/sizeof(v[0]); i++ ) {
        aa_tree_insert(t, &v[i]);
    }

    __print_u32_digraph(t, "G1");
    fprintf(stdout, "\n");

    uint32_t tmp = 0;

    tmp = 50;
    aa_tree_remove(t, &tmp);
    __print_u32_digraph(t, "G2");
    fprintf(stdout, "\n");

    aa_tree_destroy(t);
}


void test_aa_tree_remove_1_1(void) {

    char mem[aa_tree_size];

    struct aa_tree *t = aa_tree_create( sizeof(mem)
                                      , mem
                                      , sizeof(uint32_t)
                                      , __u32_less
                                      , __u32_eq
                                      , __u32_cpy
                                      , 0
                                      , __alloc
                                      , __dealloc
                                      );

    uint32_t v[] = { 50, 40 };
    size_t i = 0;

    for(; i < sizeof(v)/sizeof(v[0]); i++ ) {
        aa_tree_insert(t, &v[i]);
    }

    __print_u32_digraph(t, "G");
    fprintf(stdout, "\n");

    uint32_t tmp = 0;

    tmp = 40;
    aa_tree_remove(t, &tmp);
    __print_u32_digraph(t, "G6");
    fprintf(stdout, "\n");

    aa_tree_destroy(t);
}


void test_aa_tree_remove_1_2(void) {

    char mem[aa_tree_size];

    struct aa_tree *t = aa_tree_create( sizeof(mem)
                                      , mem
                                      , sizeof(uint32_t)
                                      , __u32_less
                                      , __u32_eq
                                      , __u32_cpy
                                      , 0
                                      , __alloc
                                      , __dealloc
                                      );

    uint32_t v[] = { 50, 40, 30, 35 };
    size_t i = 0;

    for(; i < sizeof(v)/sizeof(v[0]); i++ ) {
        aa_tree_insert(t, &v[i]);
    }

    __print_u32_digraph(t, "G");
    fprintf(stdout, "\n");

    uint32_t tmp = 0;

    tmp = 40;
    aa_tree_remove(t, &tmp);
    __print_u32_digraph(t, "G6");
    fprintf(stdout, "\n");

    aa_tree_destroy(t);
}


void test_aa_tree_remove_2(void) {

    char mem[aa_tree_size];

    struct aa_tree *t = aa_tree_create( sizeof(mem)
                                      , mem
                                      , sizeof(uint32_t)
                                      , __u32_less
                                      , __u32_eq
                                      , __u32_cpy
                                      , 0
                                      , __alloc
                                      , __dealloc
                                      );

    uint32_t v[] = { 50, 40, 100, 45, 20, 47, 15, 30, 25, 37, 27, 90 };
    size_t i = 0;

    for(; i < sizeof(v)/sizeof(v[0]); i++ ) {
        aa_tree_insert(t, &v[i]);
    }

    __print_u32_digraph(t, "G");
    fprintf(stdout, "\n");

    uint32_t rm[] = { 90, 20, 45, 30, 40  };

    for(i = 0; i < sizeof(rm)/sizeof(rm[0]); i++ ) {
        char tmp[16];
        snprintf(tmp, sizeof(tmp), "G%d", (int)i);
        fprintf(stdout, "// remove %u\n", rm[i]);
        aa_tree_remove(t, &rm[i]);
        __print_u32_digraph(t, tmp);
        fprintf(stdout, "\n");
    }

    aa_tree_destroy(t);
}

void test_aa_tree_remove_3(void) {

    char mem[aa_tree_size];

    struct aa_tree *t = aa_tree_create( sizeof(mem)
                                      , mem
                                      , sizeof(uint32_t)
                                      , __u32_less
                                      , __u32_eq
                                      , __u32_cpy
                                      , 0
                                      , __alloc
                                      , __dealloc
                                      );

    uint32_t v[] = { 4,10,2,6,12,3,1,8,13,11,5,9,7 };
    size_t i = 0;

    for(; i < sizeof(v)/sizeof(v[0]); i++ ) {
        aa_tree_insert(t, &v[i]);
    }

    __print_u32_digraph(t, "G");
    fprintf(stdout, "\n");

    uint32_t rm[] = { 1, 5 };

    for(i = 0; i < sizeof(rm)/sizeof(rm[0]); i++ ) {
        char tmp[16];
        snprintf(tmp, sizeof(tmp), "G%d", (int)i);
        fprintf(stdout, "// remove %u\n", rm[i]);
        aa_tree_remove(t, &rm[i]);
        __print_u32_digraph(t, tmp);
        fprintf(stdout, "\n");
    }

    aa_tree_destroy(t);
}

void test_aa_tree_lookup_1(void) {

    ranctx rctx;
    raninit(&rctx, 0xDEADBEEF);

    char mem[aa_tree_size];

    struct aa_tree *t = aa_tree_create( sizeof(mem)
                                      , mem
                                      , sizeof(uint32_t)
                                      , __u32_less_stat
                                      , __u32_eq_stat
                                      , __u32_cpy
                                      , 0
                                      , __alloc
                                      , __dealloc
                                      );

    const size_t N = (1<<18);
    size_t i = 0;

    size_t ln = 0;
    size_t en = 0;

    for(; i < N; i++ ) {
        uint32_t tmp = ranval(&rctx) % N;

        __less_num = 0;
        __eq_num = 0;

        aa_tree_insert(t, &tmp);

        ln = __less_num > ln ? __less_num : ln;
        en = __eq_num > en ? __eq_num : en;
    }


    fprintf( stdout
           , "inserted %zu, max. less: %zu ops, max. eq: %zu ops\n"
           , i
           , ln
           , en );


    ln = 0;
    en = 0;

    size_t found = 0;
    for(i = 0; i < N; i++ ) {
        uint32_t tmp = ranval(&rctx) % N;

        __less_num = 0;
        __eq_num = 0;

        if( aa_tree_find(t, &tmp) ) {
            found++;
        }

        ln = __less_num > ln ? __less_num : ln;
        en = __eq_num > en ? __eq_num : en;

    }

    fprintf( stdout
           , "inserted %zu, max. less: %zu ops, max. eq: %zu ops\n"
           , i
           , ln
           , en );

    fprintf(stdout, "found %zu\n", found);

    aa_tree_destroy(t);
}


void test_aa_tree_clinical_1(void) {

    ranctx rctx;
    raninit(&rctx, 0xDEADBEEF);

    char mem[aa_tree_size];

    fprintf(stdout, "clinical case\n");

    struct aa_tree *t = aa_tree_create( sizeof(mem)
                                      , mem
                                      , sizeof(uint32_t)
                                      , __u32_less_stat
                                      , __u32_eq_stat
                                      , __u32_cpy
                                      , 0
                                      , __alloc
                                      , __dealloc
                                      );

    const size_t N = 1000000;
    size_t i = 0;

    size_t ln = 0;
    size_t en = 0;

    for(; i < N; i++ ) {
        uint32_t tmp = i;

        __less_num = 0;
        __eq_num = 0;

        aa_tree_insert(t, &tmp);

        ln = __less_num > ln ? __less_num : ln;
        en = __eq_num > en ? __eq_num : en;
    }

    fprintf( stdout
           , "inserted %zu, max. less: %zu ops, max. eq: %zu ops\n"
           , i
           , ln
           , en );


    ln = 0;
    en = 0;

    size_t found = 0;
    for(i = 0; i < N; i++ ) {
        uint32_t tmp = ranval(&rctx) % N;

        __less_num = 0;
        __eq_num = 0;

        if( aa_tree_find(t, &tmp) ) {
            found++;
        }

        ln = __less_num > ln ? __less_num : ln;
        en = __eq_num > en ? __eq_num : en;

    }

    fprintf( stdout
           , "inserted %zu, max. less: %zu ops, max. eq: %zu ops\n"
           , i
           , ln
           , en );

    fprintf(stdout, "found %zu\n", found);

    aa_tree_destroy(t);
}


