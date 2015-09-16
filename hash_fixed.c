#include <assert.h>

#include "hash_fixed.h"
#include "slist.h"
#include "const_mem_pool.h"
#include "miscdata.h"

#include <stdio.h>

struct slist_allocator {
    struct const_mem_pool *pool;
    slist *free;
    size_t size;
};

size_t hash_mem_size_fixed(size_t bkt, size_t n, size_t k, size_t v) {
    return   sizeof(struct const_mem_pool)
           + sizeof(struct slist_allocator)
           + hash_minimal_mem_size(bkt, n, k, v);
}

static void *__alloc_slist(void *cc, size_t n) {

    struct slist_allocator *sa = cc;

    if( sa->free && n <= sa->size ) {
        void *mem = slist_uncons(&sa->free);
        return mem;
    } else {
        return const_mem_pool_alloc(sa->pool, MAX(sa->size, n));
    }

    return 0;
}

static void __dealloc_slist(void *cc, void *mem) {
    struct slist_allocator *sa = cc;
    sa->free = slist_cons(mem, sa->free);
}

struct hash *hash_create_fixed( size_t size
                              , void *mem
                              , size_t keysize
                              , size_t valsize
                              , size_t nbuckets
                              , uint32_t (*hashfun)(void *)
                              , bool     (*keycmp)(void *, void *)
                              , void     (*keycopy)(void *, void *)
                              , void     (*valcopy)(void *, void *) ) {


    const size_t ssz = sizeof(struct slist_allocator);

    void *pool = const_mem_pool_create(size, mem);

    if( !pool ) {
        return 0;
    }

    void *hmem = const_mem_pool_alloc(pool, hash_size);

    if( !hmem ) {
        return 0;
    }

    struct slist_allocator *ac = const_mem_pool_alloc(pool, ssz);

    if( !ac ) {
        return 0;
    }

    size_t ch = hash_chunk_size(keysize, valsize);

    assert(ch >= slist_size(0));

    ac->pool = pool;
    ac->size = ch;
    ac->free = 0;

    struct hash *h = hash_create( hash_size
                                , hmem
                                , keysize
                                , valsize
                                , nbuckets
                                , hashfun
                                , keycmp
                                , keycopy
                                , valcopy
                                , ac
                                , __alloc_slist
                                , __dealloc_slist );

    // never rehash fixed hash
    hash_set_rehash_values(h, 0, 0);
    return h;
}


