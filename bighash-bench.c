#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bighash.h"

#include "prng.h"

static void *__alloc(void *cc, size_t n) {
    void *mem = malloc(n);
/*    fprintf(stderr, "alloc %p\n", mem);*/
    return mem;
}

static void __dealloc(void *cc, void *mem) {
/*    fprintf(stderr, "free %p\n", mem);*/
    free(mem);
}

static uint32_t __u32_hash(void *a) {
    return *(uint32_t*)a;
}

static bool __u32_eq(void *a, void *b) {
    return *(uint32_t*)a == *(uint32_t*)b;
}

static void __u32_cpy(void *a, void *b) {
    *(uint32_t*)a = *(uint32_t*)b;
}


static void __create_print_1(void *cc, void *k, void *v) {
    fprintf(stdout, "(%u,%u)\n", *(uint32_t*)k, *(uint32_t*)v);
    if( cc ) {
        (*(size_t*)cc)++;
    }
}

static void __create_alter_1(void *cc, void *k, void *v, bool n) {
    *(uint32_t*)v = 0;
    fprintf( stdout
           , "alter: %s (%u,%u)\n"
           , n ? "new" : "old"
           , *(uint32_t*)k
           , *(uint32_t*)v
           );
}

static void __create_alter_2(void *cc, void *k, void *v, bool n) {
    if( !n ) {
        (*(uint32_t*)v) ++;
    }
}

static bool __create_1_less_than(void *cc, void *v, void *k) {
    return *(uint32_t*)v < *(uint32_t*)cc;
}

void test_hash_create_1(void) {

    char mem[hash_size];

    struct hash *c = hash_create( sizeof(mem)
                                , mem
                                , sizeof(uint32_t)
                                , sizeof(uint32_t)
                                , 16
                                , __u32_hash
                                , __u32_eq
                                , __u32_cpy
                                , __u32_cpy
                                , 0
                                , __alloc
                                , __dealloc
                                );

    const size_t N = 10;
    size_t i = 0;

    for(; i < N; i++ ) {
        uint32_t k = i;
        uint32_t v = 1;
        hash_add(c, &k, &v);
    }

    hash_enum(c, 0, __create_print_1);

    for(i = 0; i < N; i++ ) {
        uint32_t k = i;
        void *v = hash_get(c, &k);
        fprintf(stdout, "%zu: (%u, %u)\n", i, k, v?*(uint32_t*)v:-1);
    }

    size_t j = 0;
    for(; j < 3; j++ ) {
        for(i = 0; i < N; i++ ) {
            hash_del(c, &i);
            size_t cnt = 0;
            hash_enum(c, &cnt, __create_print_1);
            if( cnt ) {
                fprintf(stdout, "---\n");
            }
        }
    }

    fprintf(stderr, "alter\n");

    for(i = 100; i < 1000; i += 50 ) {
        uint32_t k = i;
        hash_alter(c, true, &k, 0, __create_alter_1);
    }

    fprintf(stdout, "---\n");
    hash_enum(c, 0, __create_print_1);

    for(i = 100; i < 1000; i += 50 ) {
        uint32_t k = i;
        hash_alter(c, false, &k, 0, __create_alter_2);
    }

    fprintf(stdout, "---\n");
    hash_enum(c, 0, __create_print_1);

    size_t le = 500;
    hash_filter(c, &le, __create_1_less_than);

    fprintf(stdout, "---\n");
    hash_enum(c, 0, __create_print_1);

    hash_destroy(c);
}

static void __u32_val_succ(void *cc, void *k, void *v, bool n) {

    if( !n ) {
        (*(uint32_t*)v) ++;
    } else {
        *(uint32_t*)v = 1;
    }

}

static bool __filt_even_val_1(void *cc, void *k, void *v) {
    return !(*(uint32_t*)v % 2);
}

void test_hash_create_2(void) {

    char mem[hash_size];

    struct hash *c = hash_create( sizeof(mem)
                                , mem
                                , sizeof(uint32_t)
                                , sizeof(uint32_t)
                                , 16
                                , __u32_hash
                                , __u32_eq
                                , __u32_cpy
                                , __u32_cpy
                                , 0
                                , __alloc
                                , __dealloc
                                );


    uint32_t k = 1, v = 10;
    hash_add(c, &k, &v);
    hash_add(c, &k, &v);
    hash_add(c, &k, &v);
    hash_add(c, &k, &v);

    k = 2; v = 20;
    hash_add(c, &k, &v);
    hash_add(c, &k, &v);

    k = 3; v = 30;
    hash_add(c, &k, &v);

    hash_enum(c, 0, __create_print_1);

    k = 1;
    hash_alter(c, false, &k, 0, __u32_val_succ);

    k = 2;
    hash_alter(c, false, &k, 0, __u32_val_succ);

    fprintf(stdout, "\n");
    hash_enum(c, 0, __create_print_1);

    hash_filter(c, 0, __filt_even_val_1);

    fprintf(stdout, "\n");
    hash_enum(c, 0, __create_print_1);

    hash_destroy(c);
}

#define CSTRING_MAX 128

static uint32_t __cstring_hash(void *k)
{
    char *s = k;
    uint32_t hash = 5381;
    int c = 0;

    while( (c = *s++) != '\0' ) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

static bool __cstring_eq(void *a, void *b) {
    return 0 == strncmp(a, b, CSTRING_MAX);
}

static void __cstring_cpy(void *a, void *b) {
    strncpy(a, b, CSTRING_MAX);
}

static void __u64_cpy(void *a, void *b) {
    *(uint64_t*)a = *(uint64_t*)b;
}

static void __create_3_print(void *c, void *k, void *v) {
    fprintf(stdout, "(%s,%zu)\n", (char*)k, *(uint64_t*)v);
}

static void __create_3_set_value(void *c, void *k, void *v, bool n) {
    *(uint64_t*)v = *(uint64_t*)c;
}

void test_hash_create_3(void) {

    char mem[hash_size];

    struct hash *c = hash_create( sizeof(mem)
                                , mem
                                , CSTRING_MAX
                                , sizeof(uint64_t)
                                , 16
                                , __cstring_hash
                                , __cstring_eq
                                , __cstring_cpy
                                , __u64_cpy
                                , 0
                                , __alloc
                                , __dealloc
                                );

    struct {
        char *k;
        uint64_t v;
    } kvs[] = { { "AAAAA", 21 }
              , { "EBEB", 234 }
              , { "EBEB", 500 }
              , { "zzz", 10 }
              , { "mooo", 12 }
              };

    size_t i = 0;
    for(; i < sizeof(kvs)/sizeof(kvs[0]); i++ ) {
        hash_add(c, kvs[i].k, &kvs[i].v);
    }

    hash_enum(c, 0, __create_3_print);

    uint64_t n = 0;
    hash_alter(c, false, "EBEB", &n, __create_3_set_value);

    fprintf(stdout, "\n");
    hash_enum(c, 0, __create_3_print);

    hash_destroy(c);
}


static void* __rehash_alloc(void *cc, size_t n) {
    return malloc(n);
}

static void __rehash_dealloc(void *cc, void *mem) {
    return free(mem);
}

static uint32_t __rehash_u32_hash(void *a) {
    return *(uint32_t*)a;
}


static uint32_t __rehash_u64_hash(void *a) {

    uint64_t v = *(uint64_t*)a;

/*    uint32_t hash = 0x01010101;*/

/*    hash ^= (v >> 32);*/
/*    hash <<= 15;*/
/*    hash ^= ( v & 0xFFFFFFFF );*/

    return (uint32_t)v + (uint32_t)(v >> 31);
}

static void __rehash_alter_set(void *cc, void *k, void *v, bool n) {
    *(uint64_t*)v = *(uint64_t*)cc;
}

static bool __u64_eq(void *a, void *b) {
    return *(uint64_t*)a == *(uint64_t*)b;
}

static void __rehash_1_print(void *cc, void *k, void *v) {
    fprintf(stdout, "(%zu, %zu)\n", *(uint64_t*)k, *(uint64_t*)v);
}

void test_hash_rehash_1(void) {
    char mem[hash_size];

    struct hash *c = hash_create( sizeof(mem)
                                , mem
                                , sizeof(uint64_t)
                                , sizeof(uint64_t)
                                , 32
                                , __rehash_u64_hash
                                , __u64_eq
                                , __u64_cpy
                                , __u64_cpy
                                , 0
                                , __rehash_alloc
                                , __rehash_dealloc
                                );

    struct track {
        size_t altered;
        bool deleted;
        uint64_t key;
        uint64_t val;
    };

    ranctx rctx, rctx2, rctx3;
    raninit(&rctx, 0xDEADBEEF);
    raninit(&rctx2, 0xDEADBEEF);
    raninit(&rctx3, 0x01010101010);

    const size_t N = 500000;
    size_t i = 0;

    struct track *track = calloc(1, sizeof(struct track) * N);

    assert( track );

    hash_set_rehash_values(c, 75, 100);

    size_t d = 0, a = 0;

    for(; i < N; i++ ) {
        uint64_t key = ranval(&rctx);
        uint64_t val = i;

        hash_add(c, &key, &val);

        track[i].deleted = false;
        track[i].key = key;
        track[i].val = val;

        uint64_t j = ranval(&rctx3) % (i+1);

        if( j == i || track[j].deleted ) {
            continue;
        }

        if( !(ranval(&rctx3) % 5) ) {
            track[j].val *= 100;
            hash_alter( c
                      , false
                      , &track[j].key
                      , &track[j].val
                      , __rehash_alter_set);

            if( !track[j].altered ) {
                track[j].altered++;
                a++;
            }
        } else if( !(ranval(&rctx3) % 51) ) {
            hash_del(c, &track[j].key);
            track[j].deleted = true;
            d++;
        }
    }

    size_t deleted = 0, altered = 0, diversed = 0;

    for(i = 0; i < N; i++ ) {

        uint64_t k = track[i].key;
        void *v = hash_get(c, &track[i].key);

        if( track[i].deleted ) {

            if( !v ) {
                deleted++;
            } else {
                diversed++;
            }

        } else if( v ) {
            if( !__u64_eq(v, &track[i].val) ) {
                diversed++;
            }

            uint64_t ii = i;
            if( !__u64_eq(v, &ii) ) {
                altered++;
            }

        } else {
            diversed++;
        }
    }

    fprintf( stdout
           , "deleted: %zu (%zu), altered %zu (%zu) diversed: %zu\n"
           , deleted
           , d
           , altered
           , a
           , diversed
           );

    hash_rehash_end(c);

    size_t cap = 0, used = 0, cls = 0, maxb = 0;
    hash_stats(c, &cap, &used, &cls, &maxb);

    fprintf( stdout
           , "capacity:         %zu\n"
             "used:             %zu\n"
             "collisions (avg): %zu\n"
             "max. row:         %zu\n"
           , cap
           , used
           , cls
           , maxb
           );

    free(track);
    hash_destroy(c);
}

int main(void) {
/*    test_hash_create_1();*/
/*    test_hash_create_2();*/
/*    test_hash_create_3();*/
    test_hash_rehash_1();
    return 0;
}

