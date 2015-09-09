#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "aa_map.h"
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


bool __filt_even(void *cc, void *k, void *v) {
    return !(*(uint32_t*)v % 2);
}

void test_aa_map_filter_1(void) {
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

    const size_t N = 10;
    size_t i = 0;

    for(; i < N; i++ ) {
        uint32_t tmp = i;
        aa_map_add(m, &tmp, &tmp);
    }

    aa_map_enum(m, 0, __map_print_u32);
    fprintf(stdout, "\n");

    aa_map_filter(m, 0, __filt_even);
    aa_map_enum(m, 0, __map_print_u32);
    fprintf(stdout, "\n");

    fprintf(stdout, "wipe all\n");
    aa_map_filter(m, 0, 0);

    size_t found = 0;
    for(i = 0; i < N; i++ ) {
        if( aa_map_find(m, &i) ) {
            found++;
        }
    }

    fprintf(stdout, "found: %zu\n", found);

    aa_map_enum(m, 0, __map_print_u32);
    fprintf(stdout, "\n");

    aa_map_destroy(m);
}

#define ARBITRARY_LEN 32

static size_t __s32_cmp_n = 0;
static size_t __s32_cpy_n = 0;

static int __s32_cmp(void *a, void *b) {
    __s32_cmp_n++;
    return strncmp(a, b, 31);
}

static void __s32_cpy(void *a, void *b) {
    __s32_cpy_n++;
    strncpy(a, b, 31);
}

static void __s32_print(void *cc, void *k, void *v) {
    fprintf(stdout, "(%s,%s)\n", (char*)k, (char*)v);
}

static bool __s32_shorter(void *cc, void *k, void *v) {
    return strlen(k) <= *(size_t*)cc;
}

void test_aa_map_arbitrary_kv_1(void) {

    char mem[aa_map_size];

    struct aa_map *m = aa_map_create( sizeof(mem)
                                    , mem
                                    , ARBITRARY_LEN
                                    , ARBITRARY_LEN
                                    , __s32_cmp
                                    , __s32_cpy
                                    , __s32_cpy
                                    , 0
                                    , __alloc
                                    , __dealloc
                                    );



    __s32_cmp_n = 0;
    __s32_cpy_n = 0;

    aa_map_add(m, "QQQ", "bebebe");

    fprintf( stdout
           , "adding QQQ: cmp:%zu cpy:%zu\n"
           , __s32_cmp_n
           , __s32_cpy_n);


    __s32_cmp_n = 0;
    __s32_cpy_n = 0;

    aa_map_add(m, "LALA", "fa");

    fprintf( stdout
           , "adding LALA: cmp:%zu cpy:%zu\n"
           , __s32_cmp_n
           , __s32_cpy_n);

    fprintf(stdout, "\n");

    aa_map_add(m, "0", "zero");

    aa_map_enum(m, 0, __s32_print);

    __s32_cmp_n = 0;
    __s32_cpy_n = 0;

    aa_map_del(m, "QQQ");

    fprintf( stdout
           , "\nremoving QQQ: cmp:%zu cpy:%zu\n"
           , __s32_cmp_n
           , __s32_cpy_n);

    aa_map_enum(m, 0, __s32_print);

    aa_map_destroy(m);
}

void test_aa_map_arbitrary_kv_2(void) {

    struct { char *k;
             char *v;
           } kvs[] = { { "A", "B"}
                     , { "Ababa", "DEADBEEF"}
                     , { "Q", "q"}
                     , { "Z", "zuzuz"}
                     , { "ABAKAN", "KANOBU"}
                     , { "ABAKAN", "KANOBU"}
                     , { "", "empty string"}
                     , { "KK", "some random value, whatever"}
                     , { "empty-val", ""}
                     };

    char mem[aa_map_size];

    struct aa_map *m = aa_map_create( sizeof(mem)
                                    , mem
                                    , ARBITRARY_LEN
                                    , ARBITRARY_LEN
                                    , __s32_cmp
                                    , __s32_cpy
                                    , __s32_cpy
                                    , 0
                                    , __alloc
                                    , __dealloc
                                    );

    size_t i = 0;
    for(; i < sizeof(kvs)/sizeof(kvs[0]); i++ ) {
        aa_map_add(m, kvs[i].k, kvs[i].v);
    }

    fprintf(stdout, "\n");
    aa_map_enum(m, 0, __s32_print);

    size_t n = 2;

    fprintf(stdout, "\nfilter key length > %zu\n", n);

    aa_map_filter(m, &n, __s32_shorter);

    aa_map_enum(m, 0, __s32_print);

    aa_map_destroy(m);
}

static char *randstr(ranctx *rnd, char *dst, size_t n, const char *dict) {
    char *p  = dst;
    char *pe = p + n;
    size_t dlen = strlen(dict);

    for(; p < pe; p++ ) {
        *p = dict[ranval(rnd) % dlen];
    }

    *p = 0;

    return dst;
}

static void __add_to_another(void *cc, void *k, void *v) {
    aa_map_add(cc, k, v);
}

struct lookup_kv3_cc {
    void *m;
    size_t n;
    size_t match;
};

void __lookup_kv_3(void *cc_, void *k, void *v) {
    struct lookup_kv3_cc *cc = cc_;
    if( aa_map_find(cc->m, k) ) {
        cc->match++;
    }
    cc->n++;
}

void test_aa_map_arbitrary_kv_3(void) {

    char mem[aa_map_size];

    struct aa_map *m = aa_map_create( sizeof(mem)
                                    , mem
                                    , ARBITRARY_LEN
                                    , ARBITRARY_LEN
                                    , __s32_cmp
                                    , __s32_cpy
                                    , __s32_cpy
                                    , 0
                                    , __alloc
                                    , __dealloc
                                    );

    struct aa_map *m2 = aa_map_create( sizeof(mem)
                                     , mem
                                     , ARBITRARY_LEN
                                     , ARBITRARY_LEN
                                     , __s32_cmp
                                     , __s32_cpy
                                     , __s32_cpy
                                     , 0
                                     , __alloc
                                     , __dealloc
                                     );


    ranctx rctx;
    raninit(&rctx, 0xDEADBEEF);

    const size_t N = 1000;
    size_t i = 0;

    const char dict[] = "qwertyuiopasdfghjklzxcvbnm"
                        "QWERTYUIOPASDFGHJKLZXCVBNM1234567890";

    for(; i < N; i++ ) {
        size_t ln1 = 1 + ranval(&rctx) % (ARBITRARY_LEN-2);
        size_t ln2 = 1 + ranval(&rctx) % (ARBITRARY_LEN-2);
        char k[ARBITRARY_LEN];
        char v[ARBITRARY_LEN];

        randstr(&rctx, k, ln1, dict);
        randstr(&rctx, v, ln2, dict);

        aa_map_add(m, k, v);

    }

    aa_map_enum(m, m2, __add_to_another);

    __s32_cpy_n = 0;
    __s32_cmp_n = 0;

    fprintf(stdout, "\n");

    char ks[] = "SOME STRING";
    void *v = aa_map_find(m, ks);

    fprintf(stdout
           , "found '%s' in m: %s, cmp: %zu\n"
           , ks
           , v ? (char*)v : "no"
           , __s32_cmp_n
           );


    __s32_cpy_n = 0;
    __s32_cmp_n = 0;

    v = aa_map_find(m2, ks);

    fprintf(stdout
           , "found '%s' in m2: %s, cmp: %zu\n"
           , ks
           , v ? (char*)v : "no"
           , __s32_cmp_n
           );

    size_t n = 4;
    aa_map_filter(m2, &n, __s32_shorter);

    aa_map_enum(m2, 0, __s32_print);

    __s32_cpy_n = 0;
    __s32_cmp_n = 0;

    struct lookup_kv3_cc cc = { .m = m, .n = 0, .match = 0 };

    aa_map_enum(m2, &cc, __lookup_kv_3);

    fprintf( stdout
           , "lookup in m, %zu, found %zu, avg. cmp %zu (%zu)\n"
           , cc.n
           , cc.match
           , __s32_cmp_n / cc.n
           , __s32_cmp_n
           );

    aa_map_destroy(m);
    aa_map_destroy(m2);
}




