#include "hash.h"

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

uint32_t u64hash(void *k) {
    uint64_t v = *(uint64_t*)k;
    uint32_t h = v;
    return h + ((h >> 2) | (h << 30));
}

bool u64cmp(void *a, void *b) {
    return *(uint64_t*)a == *(uint64_t*)b;
}

void u64cpy(void *dst, void *k) {
    memcpy(dst, k, sizeof(uint64_t));
}

void test_hash_create_1(void) {
    static char mem[4096];
    struct hash *c = hash_create( mem
                                , sizeof(mem)
                                , sizeof(uint64_t)
                                , sizeof(uint64_t)
                                , u64hash
                                , u64cmp
                                , u64cpy
                                , u64cpy );
    fprintf(stdout, "??? hash create %s\n", c ? "succeed" : "failed");

    if( !c ) return;

    uint64_t i = 0;
    for(;; i++) {
        uint64_t tmp = i + 10000;
        if( !hash_add(c, &i, &tmp) ) break;
    }

    fprintf(stdout, "??? hash items added: %u\n", (unsigned int)i);

    uint64_t j = 0;
    for(j = 0; j < i; j++ ) {
        uint64_t *v = hash_get(c, &j);
        if( v ) {
            fprintf(stdout, "??? hash get: %u -> %u\n", j, *v);
        } else {
            fprintf(stdout, "!!! hash get: %u -> NONE\n", j);
        }
    }


    fprintf(stdout, "??? del even values\n");

    for(j = 0; j < i; j+= 2 ) {
        hash_del(c, &j);
    }

    for(j = 0; j < i; j++ ) {
        uint64_t *v = hash_get(c, &j);
        if( v ) {
            fprintf(stdout, "??? hash get: %ul -> %u\n", j, *v);
        } else {
            fprintf(stdout, "??? hash get: %ul -> NONE\n", j);
        }
    }

    fprintf(stdout, "??? add even values\n");

    for(j = 0; j < i; j+= 2 ) {
        uint64_t tmp = j + 20000;
        hash_add(c, &j, &tmp);
    }

    for(j = 0; j < i; j++ ) {
        uint64_t *v = hash_get(c, &j);
        if( v ) {
            fprintf(stdout, "??? hash get: %ul -> %u\n", j, *v);
        } else {
            fprintf(stdout, "!!! hash get: %ul -> NONE\n", j);
        }
    }
}

static uint32_t shash(void *k) {
    char *s = k;
    uint32_t hash = 5381;
    int c = 0;
    while( (c = *s++) ) hash = ((hash << 5) + hash) + c;
    return hash;
}

static bool scmp32(void *k0, void *k1) {
    return (0 == strncmp(k0, k1, 32));
}

static void scpy32(void *dst, void *src) {
    strncpy(dst, src, 32);
}

void test_hash_create_2(void) {
    static char mem[4096];
    char tmp[32];
    struct hash *c = hash_create( mem
                                , sizeof(mem)
                                , sizeof(char[32])
                                , sizeof(uint64_t)
                                , shash
                                , scmp32
                                , scpy32
                                , u64cpy);

    size_t i = 0;
    for(;;i++) {
        snprintf(tmp, sizeof(tmp), "KEY-%04x", i);
        if( !hash_add(c, tmp, &i) ) break;
        fprintf(stdout, "??? %s %ul\n", tmp, i);
    }
    fprintf(stdout, "??? entries added: %ul\n", i);
    size_t j = 0;
    for(; j < i; j++) {
        snprintf(tmp, sizeof(tmp), "KEY-%04x", j);
        uint64_t  *v = hash_get(c, tmp);
        if( v ) {
            fprintf(stdout, "??? Found: %s %ul\n", tmp, *v);
        } else {
            fprintf(stdout, "*** Not found: %s\n", tmp);
        }
    }
    for(j=0; j < 10; j++) {
        snprintf(tmp, sizeof(tmp), "KEY-%04x", j);
        hash_del(c, &tmp);
    }
    fprintf(stdout, "??? entries deleted: %u\n", 10);
    size_t k = 0;
    for(k = i;;k++) {
        snprintf(tmp, sizeof(tmp), "NEW-KEY-%04x", k);
        if( !hash_add(c, tmp, &k) ) break;
        fprintf(stdout, "??? %s %u\n", tmp, k);
    }
    fprintf(stdout, "??? new entries added: %u\n", (k-i));
}

static void set_num(void *cc, void *k, void *v) {
    *((size_t*)v) = *((size_t*)cc);
}

static void add_100(void *cc, void *k, void *v) {
    *((size_t*)v) += 100;
}

static void dump_item(void *cc, void *k, void *v) {
    fprintf( stdout
           , "??? %s %u\n"
           , (char*)k
           , (uint32_t)(*(uint64_t*)v)
           );
}

void test_hash_alter_1(void) {
    static char mem[2048];
    char tmp[32];
    struct hash *c = hash_create( mem
                                , sizeof(mem)
                                , sizeof(char[32])
                                , sizeof(uint64_t)
                                , shash
                                , scmp32
                                , scpy32
                                , u64cpy);


    assert( c );

    size_t i = 0;
    for(;;i++) {
        snprintf(tmp, sizeof(tmp), "K%d", i);
        if( !hash_alter(c, true, tmp, &i, set_num) ) break;
        fprintf(stdout, "??? %s %u\n", tmp, i);
    }

    fprintf(stdout, "??? enum items\n");
    hash_enum_items(c, 0, dump_item);

    fprintf(stdout, "??? add 100 to evens\n");
    size_t j = 0;
    for(;j < i + 100;j+=2) {
        snprintf(tmp, sizeof(tmp), "K%d", j);
        hash_alter(c, false, tmp, &i, add_100);
    }

    hash_enum_items(c, 0, dump_item);

    fprintf(stdout, "??? del K15 and K16\n");

    snprintf(tmp, sizeof(tmp), "K%d", 15);
    hash_del(c, tmp);

    snprintf(tmp, sizeof(tmp), "K%d", 16);
    hash_del(c, tmp);

    hash_enum_items(c, 0, dump_item);


    fprintf(stdout, "??? alter K15 and K16\n");
    size_t v = 15;
    snprintf(tmp, sizeof(tmp), "K%d", 15);
    hash_alter(c, false, tmp, &v, set_num);

    snprintf(tmp, sizeof(tmp), "K%d", 16);
    v = 16;
    hash_alter(c, true, tmp, &v, set_num);

    hash_enum_items(c, 0, dump_item);
}


