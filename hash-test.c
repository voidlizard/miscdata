#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include "hash.h"

static uint32_t u64hash(void *k)
{
    uint64_t v = *(uint64_t *) k;
    uint32_t h = v;

    return h + ((h >> 2) | (h << 30));
}

static uint32_t u64hash_bad(void *k)
{
    return 0;
}

static bool u64cmp(void *a, void *b)
{
    return *(uint64_t *) a == *(uint64_t *) b;
}

static void u64cpy(void *dst, void *k)
{
    memcpy(dst, k, sizeof(uint64_t));
}

void test_hash_create_1(void)
{
    static char mem[4096];
    struct hash *c = hash_create( mem
                                , sizeof(mem)
                                , sizeof(uint64_t)
                                , sizeof(uint64_t)
                                , u64hash
                                , u64cmp
                                , u64cpy
                                , u64cpy);

    fprintf( stdout, "??? hash create %s\n"
           , c != NULL ? "succeed" : "failed");

    if( c == NULL ) {
        return;
    }

    uint64_t i = 0;

    for( ; ; i++ ) {
        uint64_t tmp = i + 10000;

        if( !hash_add(c, &i, &tmp) ) {
            break;
        }
    }

    fprintf(stdout, "??? hash items added: %" PRId64 "\n", i);

    uint64_t j = 0;

    for( j = 0; j < i; j++ ) {
        uint64_t *v = hash_get(c, &j);

        if( v ) {
            fprintf( stdout, "??? hash get: %" PRIu64 " -> %" PRIu64 "\n"
                   , j
                   , *v);
        } else {
            fprintf(stdout, "!!! hash get: %" PRIu64 " -> NONE\n", j);
        }
    }


    fprintf(stdout, "??? del even values\n");

    for( j = 0; j < i; j+= 2 ) {
        hash_del(c, &j);
    }

    for( j = 0; j < i; j++ ) {
        uint64_t *v = hash_get(c, &j);

        if( v != NULL ) {
            fprintf( stdout, "??? hash get: %" PRIu64 " -> %" PRIu64 "\n"
                   , j
                   , *v);
        } else {
            fprintf(stdout, "??? hash get: %" PRIu64 " -> NONE\n", j);
        }
    }

    fprintf(stdout, "??? add even values\n");

    for( j = 0; j < i; j+= 2 ) {
        uint64_t tmp = j + 20000;

        hash_add(c, &j, &tmp);
    }

    for( j = 0; j < i; j++ ) {
        uint64_t *v = hash_get(c, &j);

        if( v ) {
            fprintf( stdout, "??? hash get: %" PRIu64 " -> %" PRIu64 "\n"
                   , j
                   , *v);
        } else {
            fprintf(stdout, "!!! hash get: %" PRIu64 " -> NONE\n", j);
        }
    }
}

static uint32_t shash(void *k)
{
    char *s = k;
    uint32_t hash = 5381;
    int c = 0;

    while( (c = *s++) != '\0' ) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

static bool scmp32(void *k0, void *k1)
{
    return (0 == strncmp(k0, k1, 32));
}

static void scpy32(void *dst, void *src)
{
    strncpy(dst, src, 32);
}

void test_hash_create_2(void)
{
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

    for( ; ; i++ ) {
        snprintf(tmp, sizeof(tmp), "KEY-%04zx", i);

        if( !hash_add(c, tmp, &i) ) {
            break;
        }

        fprintf(stdout, "??? %s %zu\n", tmp, i);
    }

    fprintf(stdout, "??? entries added: %zu\n", i);

    size_t j = 0;

    for( ; j < i; j++) {
        snprintf(tmp, sizeof(tmp), "KEY-%04zx", j);

        uint64_t *v = hash_get(c, tmp);

        if( v ) {
            fprintf(stdout, "??? Found: %s %" PRIu64"\n", tmp, *v);
        } else {
            fprintf(stdout, "*** Not found: %s\n", tmp);
        }
    }

    for( j = 0; j < 10; j++ ) {
        snprintf(tmp, sizeof(tmp), "KEY-%04zx", j);
        hash_del(c, &tmp);
    }

    fprintf(stdout, "??? entries deleted: %u\n", 10);

    size_t k = 0;

    for( k = i; ; k++ ) {
        snprintf(tmp, sizeof(tmp), "NEW-KEY-%04zx", k);

        if( !hash_add(c, tmp, &k) ) {
            break;
        }

        fprintf(stdout, "??? %s %zu\n", tmp, k);
    }

    fprintf(stdout, "??? new entries added: %zu\n", k - i);
}

static void set_num(void *cc, void *k, void *v)
{
    *((size_t *) v) = *((size_t *) cc);
}

static void add_100(void *cc, void *k, void *v)
{
    *((size_t *) v) += 100;
}

static void dump_item(void *cc, void *k, void *v)
{
    fprintf( stdout
           , "??? %s %" PRIu64 "\n"
           , (char *) k
           , *((uint64_t *) v));
}

void test_hash_alter_1(void)
{
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

    for( ; ; i++ ) {
        snprintf(tmp, sizeof(tmp), "K%zd", i);

        if( !hash_alter(c, true, tmp, &i, set_num) ) {
            break;
        }

        fprintf(stdout, "??? %s %zu\n", tmp, i);
    }

    fprintf(stdout, "??? enum items\n");
    hash_enum_items(c, NULL, dump_item);

    fprintf(stdout, "??? add 100 to evens\n");

    size_t j = 0;

    for( ; j < i + 100; j += 2 ) {
        snprintf(tmp, sizeof(tmp), "K%zd", j);
        hash_alter(c, false, tmp, &i, add_100);
    }

    hash_enum_items(c, NULL, dump_item);

    fprintf(stdout, "??? del K15 and K16\n");

    snprintf(tmp, sizeof(tmp), "K%d", 15);
    hash_del(c, tmp);

    snprintf(tmp, sizeof(tmp), "K%d", 16);
    hash_del(c, tmp);

    hash_enum_items(c, NULL, dump_item);


    fprintf(stdout, "??? alter K15 and K16\n");

    size_t v = 15;

    snprintf(tmp, sizeof(tmp), "K%d", 15);
    hash_alter(c, false, tmp, &v, set_num);

    snprintf(tmp, sizeof(tmp), "K%d", 16);
    v = 16;
    hash_alter(c, true, tmp, &v, set_num);

    hash_enum_items(c, NULL, dump_item);
}

static void dump_int_int(void *cc, void *k, void *v)
{
    fprintf( stdout
           , "??? %" PRIu64 " %"PRIu64"\n"
           , *(uint64_t *) k
           , *(uint64_t *) v);
}

static bool filt_even_keys(void *cc, void *k, void *v)
{
    return  (0 == (*(uint64_t *) k) % 2);
}

void test_hash_filter_1(void)
{
    static char mem[2048];
    struct hash *c = hash_create( mem
                                , sizeof(mem)
                                , sizeof(uint64_t)
                                , sizeof(uint64_t)
                                , u64hash
                                , u64cmp
                                , u64cpy
                                , u64cpy );

    fprintf(stdout, "??? hash create %s\n", c ? "succeed" : "failed");

    assert(c);

    uint64_t i = 0;

    for( ; ; i++ ) {
        uint64_t tmp = i + 1000;

        if( !hash_add(c, &i, &tmp) ) {
            break;
        }
    }

    fprintf(stdout, "??? hash items added: %" PRIu64 "\n", i);

    hash_enum_items(c, NULL, dump_int_int);

    fprintf(stdout, "??? del odd values\n");

    hash_filter_items(c, NULL, filt_even_keys);

    hash_enum_items(c, NULL, dump_int_int);
}


void test_hash_grow_1(void)
{
    static char mem[1024];
    struct hash *c = hash_create( mem
                                , sizeof(mem)
                                , sizeof(uint64_t)
                                , sizeof(uint64_t)
                                , u64hash
                                , u64cmp
                                , u64cpy
                                , u64cpy );

    fprintf(stdout, "??? hash create %s\n", c ? "succeed" : "failed");

    assert(c);

    uint64_t i = 0;

    for( ; ; i++ ) {
        uint64_t tmp = i + 1000;

        if( !hash_add(c, &i, &tmp) ) {
            break;
        }
    }

    fprintf(stdout, "??? hash items added: %" PRIu64 "\n", i);

    hash_enum_items(c, NULL, dump_int_int);

    char new_chunk[512];
    if( hash_grow(c, new_chunk, sizeof(new_chunk)) ) {
        fprintf(stdout, "hash enlarged (1) ok\n");
    }

    for( ; ; i++ ) {
        uint64_t tmp = i + 1000;

        if( !hash_add(c, &i, &tmp) ) {
            break;
        }
    }

    hash_enum_items(c, NULL, dump_int_int);

    char new_chunk_2[512];
    if( hash_grow(c, new_chunk_2, sizeof(new_chunk_2)) ) {
        fprintf(stdout, "hash enlarged (2) ok\n");
    }

    for( ; ; i++ ) {
        uint64_t tmp = i + 1000;

        if( !hash_add(c, &i, &tmp) ) {
            break;
        }
    }

    hash_enum_items(c, NULL, dump_int_int);
}


static bool filt_gt_eq(void *cc, void *k, void *v)
{
    uint64_t *c = (uint64_t*)cc;
    return  *c > (*(uint64_t *)k);
}

static void __dealloc_chunk(void *_, void *mem) {
    fprintf(stderr, "dealloc chunk %zu\n", (size_t)mem);
    free(mem);
}

void test_hash_shrink_1(void)
{
    static char mem[1024];
    struct hash *c = hash_create( mem
                                , sizeof(mem)
                                , sizeof(uint64_t)
                                , sizeof(uint64_t)
                                , u64hash
                                , u64cmp
                                , u64cpy
                                , u64cpy );

    fprintf(stdout, "??? hash create %s\n", c ? "succeed" : "failed");

    assert(c);

    uint64_t i = 0;

    for( ; ; i++ ) {
        uint64_t tmp = i + 1000;

        if( !hash_add(c, &i, &tmp) ) {
            break;
        }
    }

    fprintf(stdout, "??? hash items added: %" PRIu64 "\n", i);

    hash_enum_items(c, NULL, dump_int_int);

    uint64_t del0 = i;
    void *b = malloc(1024);
    fprintf(stderr, "allocated: %zu\n", (size_t)b);
    if( hash_grow(c, b, 1024) ) {
        fprintf(stdout, "hash enlarged (1) ok\n");
    }

    for( ; ; i++ ) {
        uint64_t tmp = i + 1000;

        if( !hash_add(c, &i, &tmp) ) {
            break;
        }
    }

    hash_enum_items(c, NULL, dump_int_int);

    void *b1 = malloc(512);
    fprintf(stderr, "allocated: %zu\n", (size_t)b1);
    if( hash_grow(c, b1, 512) ) {
        fprintf(stdout, "hash enlarged (2) ok\n");
    }

    size_t del_from = i;

    for( ; ; i++ ) {
        uint64_t tmp = i + 1000;

        if( !hash_add(c, &i, &tmp) ) {
            break;
        }
    }

    fprintf(stdout, "hash dump\n");
    hash_enum_items(c, NULL, dump_int_int);

    fprintf(stdout, "deleted items greater than %zu\n", del_from);
    hash_filter_items(c, &del_from, filt_gt_eq);
    hash_enum_items(c, NULL, dump_int_int);

    hash_shrink(c, 0, __dealloc_chunk);

    hash_filter_items(c, &del0, filt_gt_eq);
    fprintf(stdout, "deleted items greater than %zu\n", del0);
    hash_enum_items(c, NULL, dump_int_int);

    uint64_t tmp = 10;
    hash_filter_items(c, &tmp, filt_gt_eq);

    hash_shrink(c, 0, __dealloc_chunk);
    
    fprintf(stdout, "exhausted: %s\n", hash_exhausted(c) ? "yes" : "no");

    for( ; ; i++ ) {
        uint64_t tmp = i + 1000;

        if( !hash_add(c, &i, &tmp) ) {
            break;
        }
    }

    fprintf(stdout, "add some items\n");
    hash_enum_items(c, NULL, dump_int_int);
    fprintf(stdout, "exhausted: %s\n", hash_exhausted(c) ? "yes" : "no");
}


void __new_uint(void *cc, void *k, void *v) {
    *(uint64_t*)v = *(uint64_t*)cc;
}

void test_hash_bad_1(void)
{
    static char mem[4096];
    struct hash *c = hash_create( mem
                                , sizeof(mem)
                                , sizeof(uint64_t)
                                , sizeof(uint64_t)
                                , u64hash_bad
                                , u64cmp
                                , u64cpy
                                , u64cpy);

    fprintf( stdout, "??? hash create %s\n"
           , c != NULL ? "succeed" : "failed");

    if( c == NULL ) {
        return;
    }

    uint64_t i = 0;

    for( ; ; i++ ) {
        uint64_t tmp = i + 10000;

        if( !hash_alter(c, true, &i, &tmp, __new_uint) ) {
            break;
        }
        fprintf(stdout, "add: %zu %zu\n", i, tmp);
    }

    uint64_t j = 0;

    for( j = 0; j < i; j++ ) {
        uint64_t *v = hash_get(c, &j);
        fprintf(stdout, "get: %zu %zu\n", j, v ? *v : 0 );
    }

}



static void test_hash_find_1_cb(void *cc, void *e) {
    uint64_t k = *(uint64_t*)cc;
    uint64_t v = *(uint64_t*)e;
    fprintf(stdout, "hash_find(%zu): %zu\n", k, v);
}

void test_hash_find_1(void)
{
    static char mem[4096];
    struct hash *c = hash_create( mem
                                , sizeof(mem)
                                , sizeof(uint64_t)
                                , sizeof(uint64_t)
                                , u64hash_bad
                                , u64cmp
                                , u64cpy
                                , u64cpy);

    fprintf( stdout, "??? hash create %s\n"
           , c != NULL ? "succeed" : "failed");

    if( c == NULL ) {
        return;
    }

    uint64_t i = 1, j = 0;

    for( ; j < 10; j++ ) {
        hash_add(c, &i, &j);
    }


    uint64_t *v = hash_get(c, &i);
    char tmp[16] = {0};
    char *p = 0;

    if( v ) {
        snprintf(tmp, sizeof(tmp)-1, "%zu", *v);
        p = tmp;
    }

    fprintf(stdout, "hash_get(%zu): %s\n", i, p);

    hash_find(c, &i, &i, test_hash_find_1_cb);

    i = 2;
    j = 10;

    for( ; j < 50; j += 10 ) {
        hash_add(c, &i, &j);
    }

    hash_find(c, &i, &i, test_hash_find_1_cb);

}



