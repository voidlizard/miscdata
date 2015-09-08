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

static int __u32_cmp(void *a, void *b) {
    return (int)( *(uint32_t*)a - *(uint32_t*)b );
}

static void __u32_cpy(void *a, void *b) {
    *(uint32_t*)a = *(uint32_t*)b;
}

static void __print_u32_dot( void *cc
                           , struct aa_node_info *p
                           , struct aa_node_info *n) {

    char tmp[128];

    if( !p || !p->value ) {
        snprintf(tmp, sizeof(tmp), "\"(nil,0)\"");
    } else {
        snprintf(tmp, sizeof(tmp), "\"(%u,%zu)\"", *(uint32_t*)p->value, p->level);
    }

    if( n && n->value ) {
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

static size_t __cmp_num = 0;

static int __u32_cmp_stat(void *a, void *b) {
    __cmp_num++;
    return (int)( *(uint32_t*)a - *(uint32_t*)b );
}

void test_aa_tree_create_1(void) {

    char mem[aa_tree_size];

    struct aa_tree *t = aa_tree_create( sizeof(mem)
                                      , mem
                                      , sizeof(uint32_t)
                                      , __u32_cmp
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
                                      , __u32_cmp
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
                                      , __u32_cmp
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
                                      , __u32_cmp
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
                                      , __u32_cmp
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
                                      , __u32_cmp
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
                                      , __u32_cmp_stat
                                      , __u32_cpy
                                      , 0
                                      , __alloc
                                      , __dealloc
                                      );

    const size_t N = (1<<18);
    size_t i = 0;

    size_t cn = 0;

    for(; i < N; i++ ) {
        uint32_t tmp = ranval(&rctx) % N;

        __cmp_num = 0;

        aa_tree_insert(t, &tmp);

        cn = __cmp_num > cn ? __cmp_num : cn;
    }


    fprintf( stdout
           , "inserted %zu, max. cmp: %zu ops\n"
           , i
           , cn );


    cn = 0;

    size_t found = 0;
    for(i = 0; i < N; i++ ) {
        uint32_t tmp = ranval(&rctx) % N;

        __cmp_num = 0;

        if( aa_tree_find(t, &tmp) ) {
            found++;
        }

        cn = __cmp_num > cn ? __cmp_num : cn;

    }

    fprintf( stdout
           , "found %zu of %zu, max. cmp: %zu ops\n"
           , found
           , i
           , cn );

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
                                      , __u32_cmp_stat
                                      , __u32_cpy
                                      , 0
                                      , __alloc
                                      , __dealloc
                                      );

    const size_t N = 10000;
    size_t i = 0;

    size_t cn = 0;

    for(; i < N; i++ ) {
        uint32_t tmp = i;

        __cmp_num = 0;

        aa_tree_insert(t, &tmp);

        cn = __cmp_num > cn ? __cmp_num : cn;
    }

    fprintf( stdout
           , "inserted %zu, max. cmp: %zu ops\n"
           , i
           , cn );


    cn = 0;

    size_t found = 0;
    for(i = 0; i < N; i++ ) {
        uint32_t tmp = ranval(&rctx) % N;

        __cmp_num = 0;

        if( aa_tree_find(t, &tmp) ) {
            found++;
        }

        cn = __cmp_num > cn ? __cmp_num : cn;

    }

    fprintf( stdout
           , "found %zu of %zu, max. cmp: %zu ops\n"
           , found
           , i
           , cn );

    aa_tree_destroy(t);
}


static void __map_print_u32(void *c, void *k, void *v) {
    fprintf(stdout, "(%u,%u)\n", *(uint32_t*)k, *(uint32_t*)v);
}

void test_aa_map_basic_1(void) {

    char mem[aa_map_size];

    struct aa_map *m = aa_map_create( sizeof(mem)
                                    , mem
                                    , sizeof(uint32_t)
                                    , sizeof(uint32_t)
                                    , __u32_cmp
                                    , __u32_cpy
                                    , __u32_cpy
                                    , 0
                                    , __alloc
                                    , __dealloc
                                    );

    const size_t N = 20;
    uint32_t  k = 9000;
    uint32_t *v = 0;

    v = aa_map_find(m, &k);

    fprintf(stdout, "found %u? %s\n", k, v ? "yes" : "no");

    uint32_t ii = 1;
    bool ins = aa_map_add(m, &k, &ii);
    fprintf(stdout, "inserted %u? %s\n", k, ins ? "yes" : "no");

    v = aa_map_find(m, &k);
    fprintf(stdout, "found %u? %s %u\n", k, v ? "yes" : "no", v ? *v : (-1));

    fprintf(stdout, "del %u\n", k);
    aa_map_del(m, &k);
    v = aa_map_find(m, &k);
    fprintf(stdout, "found %u? %s\n", k, v ? "yes" : "no");

    for(k = 0; k < N; k++ ) {
        aa_map_add(m, &k, &k);
    }

    aa_map_enum(m, 0, __map_print_u32);

    for(k = 0; k < N; k += 2 ) {
        uint32_t *v = aa_map_find(m, &k);
        fprintf(stdout, "found %u: %u\n", k, v ? *v : -1);
    }

    aa_map_destroy(m);
}

void __u32_mul(void* cc, void *k, void *v, bool n) {
    if( n ) {
        *(uint32_t*)v = 1;
    } else {
        *(uint32_t*)v *= *(uint32_t*)cc;
    }
}

void __u32_mul_even(void* cc, void *k, void *v, bool n) {
    if( n ) {
        *(uint32_t*)v = 1;
    } else if( !(*(uint32_t*)v % 2) ) {
        *(uint32_t*)v *= *(uint32_t*)cc;
    }
}


void __u32_set(void* cc, void *k, void *v, bool n) {
    *(uint32_t*)v = *(uint32_t*)cc;
}

void test_aa_map_alter_1(void) {

    char mem[aa_map_size];

    struct aa_map *m = aa_map_create( sizeof(mem)
                                    , mem
                                    , sizeof(uint32_t)
                                    , sizeof(uint32_t)
                                    , __u32_cmp
                                    , __u32_cpy
                                    , __u32_cpy
                                    , 0
                                    , __alloc
                                    , __dealloc
                                    );


    uint32_t k = 1;
    uint32_t z = 1;
    const size_t N = 20;

    aa_map_alter(m, false, &k, &z, __u32_set);
    aa_map_enum(m, 0, __map_print_u32);
    fprintf(stdout, "\n");

    aa_map_alter(m, true, &k, &z, __u32_set);
    aa_map_enum(m, 0, __map_print_u32);
    fprintf(stdout, "\n");

    z = 10;
    aa_map_alter(m, false, &k, &z, __u32_mul);
    aa_map_enum(m, 0, __map_print_u32);
    fprintf(stdout, "\n");

    z = 10;
    aa_map_alter(m, true, &k, &z, __u32_mul);
    aa_map_enum(m, 0, __map_print_u32);
    fprintf(stdout, "\n");

    k = 2;
    aa_map_alter(m, true, &k, &z, __u32_mul);
    aa_map_enum(m, 0, __map_print_u32);
    fprintf(stdout, "\n");

    for(k = 3; k < N; k++ ) {
        aa_map_alter(m, true, &k, &k, __u32_set);
    }

    aa_map_enum(m, 0, __map_print_u32);
    fprintf(stdout, "\n");

    z = 1000;
    for(k = 3; k < N; k++ ) {
        aa_map_alter(m, true, &k, &z, __u32_mul_even);
    }

    aa_map_enum(m, 0, __map_print_u32);
    fprintf(stdout, "\n");

    aa_map_destroy(m);
}
