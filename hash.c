#include "hash.h"

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "slist.h"

#define HASH_BUCKETS 64

#define safecall(v, f, ...)  ((f) ? (f(__VA_ARGS__)) : (v))
#define unit {}

#define HASH(A) ((A) % HASH_BUCKETS)


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

    const size_t idx = HASH(safecall(0, c->hashfun, k));

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

void hash_enum_items(struct hash *c, void (*cb)(void *k, void *v)) {
    assert(c);

    size_t idx = 0;
    for( idx = 0; idx < HASH_BUCKETS; idx++ ) {
        slist *it = c->buckets[idx];
        for(; it; it = it->next ) {
            safecall( unit
                    , cb
                    , __hash_key(c, it->value)
                    , __hash_val(c, it->value));
        }
    }
}

bool hash_add(struct hash* c, void *k, void *v) {
    assert(c);

    size_t idx = HASH(safecall(0, c->hashfun, k));

    slist *it = slist_uncons(&c->free);

    if( !it ) return false;

    __hash_init_chunk(c, it->value, k, v);
    c->buckets[idx] = slist_cons(it, c->buckets[idx]);

    return true;
}

void hash_del( struct hash *c, void *k) {
    assert( c);
    size_t idx = HASH(safecall(0, c->hashfun, k));
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
