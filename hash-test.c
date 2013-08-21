#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "slist.h"

#define HASH_BUCKETS 64

#define safecall(v, f, ...)  ((f) ? (f(__VA_ARGS__)) : (v))
#define unit {}

// interface

struct hash;

struct hash* hash_create( void  *mem
                        , size_t memsize
                        , size_t keysize
                        , size_t valsize
                        , uint32_t (*hashfun)(void*)
                        , bool     (*keycmp)(void*, void*)
                        , void     (*keycopy)(void*, void*)
                        , void     (*valcopy)(void*, void*));

bool hash_add(struct hash *c, void *k, void *v);
void *hash_get(struct hash *c, void *k);

// implementation

struct hash {
    slist *buckets[HASH_BUCKETS];
    slist *free;
    uint32_t (*hashfun)(void*);
    bool     (*keycmp)(void*, void*);
    void     (*keycopy)(void*, void*);
    void     (*valcopy)(void*, void*);
    size_t   keysize;
    size_t   valsize;
    char   pool[0];
};

size_t __hash_entry_len(struct hash *c) {
    return c->keysize + c->valsize;
}

void __hash_init_chunk(struct hash *c, void *chunk, void *key, void *value) {
    char *p = chunk;
    safecall(unit, c->keycopy, &p[0], key);
    safecall(unit, c->valcopy, &p[c->keysize], value);
}

void *__hash_key(struct hash *c, void *chunk) {
    return &((char*)chunk)[0];
}

void *__hash_val(struct hash *c, void *chunk) {
    return &((char*)chunk)[c->keysize];
}

struct hash* hash_create( void  *mem
                        , size_t memsize
                        , size_t keysize
                        , size_t valsize
                        , uint32_t (*hashfun)(void*)
                        , bool     (*keycmp)(void*, void*)
                        , void     (*keycopy)(void*, void*)
                        , void     (*valcopy)(void*, void*)
                        ) {

    size_t poolsz = memsize - sizeof(struct hash);

    fprintf(stdout, "pool size: %d (%d, %d, %d)\n", poolsz, memsize, sizeof(struct hash), keysize + valsize);

    if( poolsz < sizeof(struct hash) + slist_size(keysize + valsize) ) {
        return 0;
    }

    struct hash *c = (struct hash*)mem;
    c->keysize = keysize;
    c->valsize = valsize;
    c->hashfun = hashfun;
    c->keycmp  = keycmp;
    c->keycopy = keycopy;
    c->valcopy = valcopy;

    c->free = slist_pool( c->pool
                        , slist_size(__hash_entry_len(c))
                        , poolsz );

    size_t i = 0;
    for(; i < HASH_BUCKETS; i++ ) {
        c->buckets[i] = slist_nil();
    }

    return c;
}

void *hash_get(struct hash *c, void *k) {
    assert(c);

    size_t idx = c->hashfun(k) % HASH_BUCKETS;

    if( !c->buckets[idx] )
        return 0;

    slist *it = c->buckets[idx];
    for(; it; it = it->next ) {
        if( c->keycmp(k, __hash_key(c, it->value)) ) {
            return __hash_val(c, it->value);
        }
    }

    return 0;
}

bool hash_add(struct hash* c, void *k, void *v) {
    assert(c);

    fprintf(stdout, "hash_add\n");

    size_t idx = safecall(0, c->hashfun, k);

    slist *it = slist_uncons(&c->free);

    fprintf(stdout, "wtf -1: %ul\n", it);

    if( !it ) return false;

    fprintf(stdout, "wtf 0: %d %ul\n", idx, c->buckets[idx]);

    __hash_init_chunk(c, it->value, k, v);
    c->buckets[idx] = slist_cons(it, c->buckets[idx]);

    fprintf(stdout, "wtf 1: %d %ul\n", idx, c->buckets[idx]);

    return true;
}

void hash_del( struct hash *c, void *k) {
    assert( c);
    size_t idx = safecall(0, c->hashfun, k);
    slist *lnew = slist_nil();
    for(; c->buckets[idx]; ) {
        slist *it=slist_uncons(&c->buckets[idx]);
        if( c->keycmp(k, __hash_key(c, it->value)) ) {
            c->free = slist_cons(it, c->free);
        } else {
            lnew = slist_cons(it, lnew);
        }
    }
    c->buckets[idx] = lnew;
}

uint32_t u64hash(void *k) {
    fprintf(stdout, "u64hash %ul\n", *(uint64_t*)k);
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
    static char mem[8192];
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
    while( hash_add(c, &i, &i) ) {
        fprintf(stdout, "added item %d\n", i);
        i++;
    }

    fprintf(stdout, "??? hash items added: %ul\n", (unsigned int)i);

}

int main(void) {

    test_hash_create_1();

    return 0;
}

