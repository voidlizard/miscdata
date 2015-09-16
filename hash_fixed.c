#include "hash_fixed.h"
#include "const_mem_pool.h"

struct hash *hash_create_fixed( size_t size
                              , void *mem
                              , size_t keysize
                              , size_t valsize
                              , size_t nbuckets
                              , uint32_t (*hashfun)(void *)
                              , bool     (*keycmp)(void *, void *)
                              , void     (*keycopy)(void *, void *)
                              , void     (*valcopy)(void *, void *) ) {


    void *pool = const_mem_pool_create(size, mem);

    if( !pool ) {
        return 0;
    }

    return hash_create( hash_size
                      , const_mem_pool_alloc(pool, hash_size)
                      , keysize
                      , valsize
                      , nbuckets
                      , hashfun
                      , keycmp
                      , keycopy
                      , valcopy
                      , pool
                      , const_mem_pool_alloc
                      , const_mem_pool_dealloc );

}


