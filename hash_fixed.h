#ifndef __hash_fixed_h
#define __hash_fixed_h

#include "hash.h"

size_t hash_size_fixed(size_t n, size_t k, size_t v);

struct hash *hash_create_fixed( size_t size
                              , void *mem
                              , size_t keysize
                              , size_t valsize
                              , size_t nbuckets
                              , uint32_t (*hashfun)(void *)
                              , bool     (*keycmp)(void *, void *)
                              , void     (*keycopy)(void *, void *)
                              , void     (*valcopy)(void *, void *) );


#endif
