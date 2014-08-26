#include "hash.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "slist.h"

#ifndef HASH_BUCKETS
#define HASH_BUCKETS 64
#endif

#define safecall(v, f, ...) ((f) ? (f(__VA_ARGS__)) : (v))
#define unit                {}

#define HASH(A)             ((A) % HASH_BUCKETS)

// implementation

struct hash {
    size_t     keysize;
    size_t     valsize;
    slist    **buckets;
    slist     *free;
    uint32_t (*hashfun)(void *);
    bool     (*keycmp)(void *, void *);
    void     (*keycopy)(void *, void *);
    void     (*valcopy)(void *, void *);
    void      *alloc_cc;
    void     *(*alloc_fn)(void*,size_t);
    void      (*dealloc_fn)(void*,void*);
    char      *pool;
    slist     *mem_chunks;
    char      mem[0];
};

struct hash_bucket {
    uint32_t hashcode;
    char     data[0];
};

struct hash_mem_chunk {
    size_t size;
    size_t refcount;
};

static void __hash_alloc_new_chunk(struct hash *h);

static inline size_t __hash_bucket_size(size_t ksize, size_t vsize) {
    return (sizeof(struct hash_bucket) + (ksize) + (vsize));
}

static inline size_t __hash_entry_len(struct hash *c)
{
    return __hash_bucket_size(c->keysize, c->valsize);
}

static inline void *__hash_key(struct hash *c, void *chunk)
{
    struct hash_bucket *b = (struct hash_bucket*)chunk;
    return &b->data[0];
}

static inline void *__hash_val(struct hash *c, void *chunk)
{
    struct hash_bucket *b = (struct hash_bucket*)chunk;
    return &b->data[c->keysize];
}

struct hash* hash_create( void  *mem
                        , size_t memsize
                        , size_t keysize
                        , size_t valsize
                        , uint32_t (*hashfun)(void *)
                        , bool     (*keycmp)(void *, void *)
                        , void     (*keycopy)(void *, void *)
                        , void     (*valcopy)(void *, void *)
                        )
{
    const size_t buck_num     = HASH_BUCKETS;
    const size_t buckets_size = buck_num*sizeof(slist*);

    size_t poolsz = memsize - sizeof(struct hash) - buckets_size;
    const size_t bsize = slist_size(__hash_bucket_size(keysize, valsize));

    if( poolsz < sizeof(struct hash) + bsize) {
        return NULL;
    }

    struct hash *c = (struct hash *) mem;

    c->buckets = (slist**)&c->mem[0];
    c->pool    = &c->mem[buckets_size];

    c->keysize = keysize;
    c->valsize = valsize;
    c->hashfun = hashfun;
    c->keycmp  = keycmp;
    c->keycopy = keycopy;
    c->valcopy = valcopy;

    c->free = slist_pool( c->pool
                        , slist_size(__hash_entry_len(c))
                        , poolsz);

    c->mem_chunks = slist_nil();

    size_t i = 0;

    for( ; i < buck_num; i++ ) {
        c->buckets[i] = slist_nil();
    }

    c->alloc_cc = 0;
    c->alloc_fn = 0;

    return c;
}

bool hash_exhausted(struct hash *c) {
    assert(c != NULL);
    return c->free == NULL;
}

bool hash_grow(struct hash *c, void *mem, size_t memsize) {
    assert(c != NULL);

    const size_t hl = slist_size(__hash_entry_len(c));
    const size_t hdrlen = slist_size(sizeof(struct hash_mem_chunk));

    if( mem == NULL || memsize < hdrlen + hl ) {
        return false;
    }

    slist *e = mem;
    c->mem_chunks = slist_cons(e, c->mem_chunks);

    size_t poolsz = memsize - hdrlen - hl;

    slist *fl = slist_pool_(c->free, &e->value[hdrlen], hl, poolsz);

    struct hash_mem_chunk *hdr = (struct hash_mem_chunk*)&e->value;
    hdr->size = memsize;

    if( fl != NULL ) {
        c->free = fl;
        return true;
    }

    return false;
}

static void __chunk_reset_refcount(void *cc, void *ch) {
    struct hash_mem_chunk *hdr = (struct hash_mem_chunk*)ch;
    hdr->refcount = 0;
}

static void __mark_chunk(void *v, void *chunk) {
    struct hash_mem_chunk *hdr = (struct hash_mem_chunk*)chunk;

    const char *ra = (char*)chunk;
    const char *rb = ra + hdr->size;
    const char *e  = (char*)v;

    if( e > ra && e < rb ) {
        hdr->refcount++;
    }

}

static void __mark_chunks(void *ch, void *v) {
    slist_foreach((slist*)ch, v, __mark_chunk);
}

static bool __chunk_used(void *cc, void *e) {
    struct hash_mem_chunk *hdr = (struct hash_mem_chunk*)e;
    return hdr->refcount > 0;
}

static bool __not_belongs_to_deleted_chunk(void *c, void *v) {
    struct hash_mem_chunk *hdr = (struct hash_mem_chunk*)c;

    const char *ra = (char*)c;
    const char *rb = ra + hdr->size;
    const char *e  = (char*)v;

    const bool belongs = e > ra && e < rb;

    return !belongs;
}

void hash_shrink(struct hash *c, void *cc, void (*dealloc)(void*, void*)) {
    assert(c != NULL);

    slist_foreach(c->mem_chunks, 0, __chunk_reset_refcount);

    size_t idx = 0;

    for( idx = 0; idx < HASH_BUCKETS; idx++ ) {
        slist_foreach(c->buckets[idx], c->mem_chunks, __mark_chunks);
    }

    slist *to_del = slist_nil();
    slist_partition_destructive( &c->mem_chunks
                               , &to_del
                               , 0
                               , __chunk_used
                               );


    while( to_del ) {
        slist *e = slist_uncons(&to_del);

        slist_filt_destructive( &c->free
                               , e->value
                               , __not_belongs_to_deleted_chunk
                               , 0
                               , 0
                               );

        if( e && dealloc ) {
            dealloc(cc, e);
            continue;
        }
        break;
    }
}

static inline slist* __hash_find(struct hash *c, void *k)
{
    const uint32_t hash_1 = c->hashfun(k);
    slist *it = c->buckets[HASH(hash_1)];

    for( ; it; it = it->next ) {
        struct hash_bucket *b = (struct hash_bucket*)it->value;
        void *hk =__hash_key(c, it->value);
        if( hash_1 == b->hashcode ) {
            if( c->keycmp(k, hk) ) {
                return it;
            }
        }
    }

    return NULL;
}


void hash_find(struct hash *c, void *k, void *cc, void (*cb) (void*, void*) )
{
    const uint32_t hash_1 = c->hashfun(k);
    slist *it = c->buckets[HASH(hash_1)];

    for( ; it; it = it->next ) {
        struct hash_bucket *b = (struct hash_bucket*)it->value;
        void *hk =__hash_key(c, it->value);
        if( hash_1 == b->hashcode ) {
            if( c->keycmp(k, hk) ) {
                cb(cc, __hash_val(c, it->value));
            }
        }
    }
}


void *hash_get(struct hash *c, void *k)
{
    slist *it = __hash_find(c, (void *) k);
    return it ? __hash_val(c, it->value) : NULL;
}

void hash_enum_items( struct hash *c
                    , void *cc
                    , void (*cb)(void *, void *, void *))
{
    assert(c != NULL);

    size_t idx = 0;

    for( idx = 0; idx < HASH_BUCKETS; idx++ ) {
        slist *it = c->buckets[idx];

        for( ; it; it = it->next ) {
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
                      , bool (*cb)(void *, void *, void *))
{
    assert(c != NULL);

    size_t idx = 0;

    for( idx = 0; idx < HASH_BUCKETS; idx++ ) {
        slist *lnew = slist_nil();

        for( ; c->buckets[idx]; ) {
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

static inline slist *__hash_add_entry(struct hash *c, void *k)
{
    assert(c != NULL);

    const uint32_t hashcode = c->hashfun(k);

    size_t idx = HASH(hashcode);

    if( hash_exhausted(c) ) {
        __hash_alloc_new_chunk(c);
    }

    slist *it = slist_uncons(&c->free);

    if( it == NULL ) {
        return NULL;
    }

    c->buckets[idx] = slist_cons(it, c->buckets[idx]);

    struct hash_bucket *b = (struct hash_bucket*)it->value;
    b->hashcode = hashcode;

    safecall(unit, c->keycopy, __hash_key(c,it->value), k);

    return it;
}

bool hash_add(struct hash* c, void *k, void *v)
{
    slist *it = __hash_add_entry(c,k);

    if( it == NULL ) {
        return false;
    }

    safecall(unit, c->valcopy, __hash_val(c, it->value), v);

    return true;
}

void *hash_get_add(struct hash *c, void *k, void *v) {
    void *v_ = hash_get(c, k);

    if( v_ ) {
        return v_;
    }

    if( !hash_add(c, k, v) ) {
        return 0;
    }

    return hash_get(c, k);
}

void hash_del( struct hash *c, void *k)
{
    assert(c != NULL);

    size_t idx = HASH(safecall(0, c->hashfun, k));

    slist *lnew = slist_nil();

    for( ; c->buckets[idx]; ) {
        slist *it=slist_uncons(&c->buckets[idx]);

        if( c->keycmp(k, __hash_key(c, it->value)) ) {
            c->free = slist_cons(it, c->free);
        } else {
            lnew = slist_cons(it, lnew);
        }
    }

    c->buckets[idx] = lnew;
}

bool hash_alter( struct hash *c
               , bool add
               , void *k
               , void *cc
               , void (*cb) (void *, void *, void *) )
{
    assert(c != NULL);

    slist *it = __hash_find(c, k);

    if( it == NULL && add ) {
        it = __hash_add_entry(c, k);
    }

    if( it == NULL ) {
        return false;
    }

    safecall( unit
            , cb
            , cc
            , __hash_key(c, it->value)
            , __hash_val(c, it->value));

    return true;
}

bool hash_alter2( struct hash *c
                , bool add
                , void *k
                , void *cc
                , void (*cb) (void *, void *, void *, bool) )
{
    assert(c != NULL);

    slist *it = __hash_find(c, k);

    bool new_item = false;

    if( it == NULL && add ) {
        it = __hash_add_entry(c, k);
        new_item = true;
    }

    if( it == NULL ) {
        return false;
    }

    safecall( unit
            , cb
            , cc
            , __hash_key(c, it->value)
            , __hash_val(c, it->value)
            , new_item );

    return true;
}


void hash_set_autogrow( struct hash *h
                      , size_t limit
                      , void *cc
                      , void *(*alloc)(void *, size_t)
                      , void (*dealloc)(void*, void *)) {
    h->alloc_cc = cc;
    h->alloc_fn = alloc;
    h->dealloc_fn = dealloc;
}

void hash_auto_shrink(struct hash *c) {
    if( c->dealloc_fn ) {
        hash_shrink(c, c->alloc_cc, c->dealloc_fn);
    }
}

static void __hash_alloc_new_chunk(struct hash *h) {
    const size_t page_size = 4096;
    const size_t min_items = 4;
    const size_t hl = slist_size(__hash_entry_len(h));
    const size_t hdrlen = slist_size(sizeof(struct hash_mem_chunk));
    const size_t memrest = page_size - hdrlen;

    assert( page_size > hdrlen );

    const size_t n = hl*2 < memrest ? memrest/hl
                                    : min_items;

    const size_t chunk_len = hdrlen + n*hl;

    if( !h->alloc_fn || !h->dealloc_fn ) {
        return;
    }

    void *mem = h->alloc_fn(h->alloc_cc, chunk_len);
    (void)hash_grow(h, mem, chunk_len);
}

void hash_memory_info( struct hash *c
                     , size_t *chunk_hdr_size
                     , size_t *item_size
                     ) {
    *item_size = slist_size(__hash_entry_len(c));
    *chunk_hdr_size = slist_size(sizeof(struct hash_mem_chunk));
}

