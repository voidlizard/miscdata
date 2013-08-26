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

static inline slist* __hash_find(struct hash *c, void *k) {
    assert(c);

    const size_t idx = HASH(safecall(0, c->hashfun, k));

    if( !c->buckets[idx] )
        return 0;

    slist *it = c->buckets[idx];
    for(; it; it = it->next ) {
        if( c->keycmp(k, __hash_key(c, it->value)) ) {
            return it;
        }
    }

    return 0;
}

void *hash_get(struct hash *c, void *k) {
    assert(c);
    const slist *it = __hash_find(c, k);
    return it ? __hash_val(c, it->value) : 0;
}

void hash_enum_items( struct hash *c
                    , void *cc
                    , void (*cb)(void *, void *, void *)) {
    assert(c);

    size_t idx = 0;
    for( idx = 0; idx < HASH_BUCKETS; idx++ ) {
        slist *it = c->buckets[idx];
        for(; it; it = it->next ) {
            safecall( unit
                    , cb
                    , cc
                    , __hash_key(c, it->value)
                    , __hash_val(c, it->value));
        }
    }
}

void hash_filter_items( struct hash *c
                      , void *cc
                      , bool (*cb)(void *, void *, void *)) {
    assert(c);

    size_t idx = 0;
    for( idx = 0; idx < HASH_BUCKETS; idx++ ) {
        slist *lnew = slist_nil();
        for(; c->buckets[idx]; ) {
            slist *it=slist_uncons(&c->buckets[idx]);
            bool leave = safecall( false
                                 , cb
                                 , cc
                                 , __hash_key(c, it->value)
                                 , __hash_val(c, it->value));
            if( leave ) {
                lnew = slist_cons(it, lnew);
            } else {
                c->free = slist_cons(it, c->free);
            }
        }
        c->buckets[idx] = lnew;
    }
}


static inline slist *__hash_add_entry(struct hash *c, void *k) {
    assert(c);

    size_t idx = HASH(safecall(0, c->hashfun, k));

    slist *it = slist_uncons(&c->free);

    if( !it ) return 0;

    c->buckets[idx] = slist_cons(it, c->buckets[idx]);

    safecall(unit, c->keycopy, __hash_key(c,it->value), k);

    return it;
}


bool hash_add(struct hash* c, void *k, void *v) {
    slist *it = __hash_add_entry(c,k);

    if( !it ) return false;

    safecall(unit, c->valcopy, __hash_val(c,it->value), v);

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

bool hash_alter(  struct hash* c
                , bool add
                , void *k
                , void *cc
                , void (*cb) (void *, void *, void *) ) {

    assert(c);
    slist *it = __hash_find(c, k);

    if( !it && add ) {
        it = __hash_add_entry(c,k);
    }

    if( !it ) return false;

    safecall( unit
            , cb
            , cc
            , __hash_key(c, it->value)
            , __hash_val(c, it->value));

    return true;
}

