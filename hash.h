#ifndef __hash_h
#define __hash_h

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

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

bool hash_alter(  struct hash* c
                , bool add
                , void *k
                , void *ctx
                , void (*) (void*, void*, void*) );


void hash_enum_items( struct hash *c
                     , void *cc
                     , void (*)(void *, void *, void *));

#endif
