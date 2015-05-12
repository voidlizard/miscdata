#ifndef __HASH_H__
#define __HASH_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

struct hash;

struct hash *hash_create( void  *mem
                        , size_t memsize
                        , size_t keysize
                        , size_t valsize
                        , uint32_t (*hashfun)(void *)
                        , bool     (*keycmp)(void *, void *)
                        , void     (*keycopy)(void *, void *)
                        , void     (*valcopy)(void *, void *));

size_t hash_mem_size(size_t n, size_t key_len, size_t val_len);

bool hash_grow(struct hash *c, void *mem, size_t memsize);

void hash_shrink(struct hash *, void*, void (*)(void*, void*));

bool hash_exhausted(struct hash *c);

bool hash_add(struct hash *c, void *k, void *v);

void *hash_get(struct hash *c, void *k);

void hash_find( struct hash *c
              , void *k
              , void *cc
              , void (*cb) (void *cc, void *v));

void hash_del( struct hash *c, void *k);

bool hash_alter( struct hash* c
               , bool add
               , void *k
               , void *ctx
               , void (*) (void *, void *, void *) );

bool hash_alter2( struct hash *c
                , bool add
                , void *k
                , void *cc
                , void (*cb) (void *, void *, void *, bool) );

void hash_enum_items( struct hash *c
                    , void *cc
                    , void (*)(void *, void *, void *));

void hash_filter_items( struct hash *c
                      , void *cc
                      , bool (*cb)(void *, void *, void *));


void *hash_get_add(struct hash *c, void *k, void *v);


void hash_memory_info( struct hash *c
                     , size_t *chunk_hdr_size
                     , size_t *item_size
                     );

void hash_set_autogrow( struct hash*
                      , size_t
                      , void *
                      , void *(*alloc)(void *, size_t)
                      , void (*dealloc)(void*, void *));

void hash_auto_shrink(struct hash *);

#endif

