#include <string.h>

#include <stdio.h>

#include "bighash.h"

struct hash_item {
    struct hash_item *next;
    char data[0];
};

struct hash_table {
    size_t used;
    size_t capacity;
    struct hash_item *data[0];
};

struct hash {

    struct hash_table *active;
    struct hash_table *shadow;

    struct hash_table *table[2];

    size_t keysize;
    size_t valsize;
    size_t nbuckets;
    size_t rehash_rate;
    size_t rehash_move;

    struct {
        size_t i;
    } rehash;

    uint32_t (*hashfun)(void *);
    bool     (*keycmp)(void *, void *);
    void     (*keycopy)(void *, void *);
    void     (*valcopy)(void *, void *);
    void   *allocator;
    void   *(*alloc)(void*,size_t);
    void    (*dealloc)(void*,void*);
};

const size_t hash_size = sizeof(struct hash);

static inline bool hash_alloc_table(struct hash *c, size_t n, size_t buckets);
static inline void hash_set_active(struct hash *c, size_t n);
static inline void hash_set_shadow(struct hash *c, size_t n);
static inline void *hash_item_key(struct hash *c, struct hash_item *e);
static inline void *hash_item_val(struct hash *c, struct hash_item *e);
static inline size_t hash_item_size(struct hash*);

static inline size_t bucket(struct hash_table *c, size_t n) {
    return n % c->capacity;
}

struct hash *hash_create( size_t memsize
                        , void  *mem
                        , size_t keysize
                        , size_t valsize
                        , size_t nbuckets
                        , uint32_t (*hashfun)(void *)
                        , bool     (*keycmp)(void *, void *)
                        , void     (*keycopy)(void *, void *)
                        , void     (*valcopy)(void *, void *)
                        , void   *allocator
                        , void   *(*alloc)(void*,size_t)
                        , void    (*dealloc)(void*,void*)
                        ) {

    if( memsize < hash_size ) {
        return 0;
    }

    struct hash *c = mem;

    c->active = 0;
    c->shadow = 0;
    c->table[0] = 0;
    c->table[1] = 0;

    c->keysize = keysize;
    c->valsize = valsize;
    c->nbuckets = nbuckets;
    c->hashfun = hashfun;
    c->keycmp = keycmp;
    c->keycopy = keycopy;
    c->valcopy = valcopy;
    c->allocator = allocator;
    c->alloc = alloc;
    c->dealloc = dealloc;

    c->rehash_rate = 75;
    c->rehash_move = 256;

    c->rehash.i = 0;

    if( !hash_alloc_table(c, 0, c->nbuckets) ) {
        return 0;
    }

    hash_set_active(c, 0);

    return c;
}

void hash_set_rehash_values(struct hash *c, size_t r, size_t n) {
    c->rehash_rate = r;
    c->rehash_move = n;
}

void hash_destroy(struct hash *c) {
    size_t h = 0;
    for(h; h < 2; h++ ) {

        struct hash_table *t = c->table[h];

        if( !t ) {
            continue;
        }

        size_t i = 0;
        for(; i < t->capacity; i++ ) {
            struct hash_item *e = t->data[i];
            while(e) {
                void *zombie = e;
                e = e->next;
                c->dealloc(c->allocator, zombie);
            }
        }
    }

    if( c->table[0] ) {
        c->dealloc(c->allocator, c->table[0]);
    }
    if( c->table[1] ) {
        c->dealloc(c->allocator, c->table[1]);
    }
}

static struct hash_item *hash_item_create(struct hash *c, void *k, void *v) {
    struct hash_item *e = c->alloc(c->allocator, hash_item_size(c));
    e->next = 0;
    c->keycopy(hash_item_key(c,e), k);

    if( v ) {
        c->valcopy(hash_item_val(c,e), v);
    }

    return e;
}

static void hash_table_add(struct hash_table *t, size_t n, struct hash_item *e) {
    const size_t i = bucket(t, n);
    e->next = t->data[i];

    if( !t->data[i] && e ) {
        t->used++;
    }

    t->data[i] = e;
}

static inline struct hash_table *to_add(struct hash *c) {
    return c->shadow ? c->shadow : c->active;
}

static inline hash_rehash_step(struct hash *c) {

    if( !c->shadow ) {
        return;
    }

    size_t i = 0, moved = 0;

    size_t j = c->rehash.i;

    for(; moved < c->rehash_move && j < c->active->capacity; ) {

        if( !c->active->data[j] ) {
            j++;
            continue;
        }

        struct hash_item *e = c->active->data[j];
        c->active->data[j] = e->next;

        uint32_t hc = c->hashfun(hash_item_key(c, e));
        hash_table_add(c->shadow, hc, e);
        moved++;
    }

    c->rehash.i = j;

    if( !moved ) {
        c->dealloc(c->allocator, c->active);
        c->active = c->shadow;
        c->shadow = 0;
        c->table[0] = c->active;
        c->table[1] = c->shadow;
        c->rehash.i = 0;
    }

}

static void hash_rehash_start(struct hash *c) {

    // FIXME
    uint64_t used = c->active->used * 100;
    uint64_t capacity = c->active->capacity;
    uint64_t r = used / capacity;

    if( c->shadow || r < c->rehash_rate ) {
        return;
    }

    size_t i = 1;

    i = c->active == c->table[0] ? 1 : 0;

    // FIXME: various methods
    size_t buckets = c->active->capacity * 2;

    if( !hash_alloc_table(c, i, buckets) ) {
        // FIXME: error handling
        return;
    }

    c->shadow = c->table[i];
    c->rehash.i = 0;
}

void hash_rehash_end(struct hash *c) {
    while( c->shadow ) {
        hash_rehash_step(c);
    }
}

bool hash_add(struct hash *c, void *k, void *v) {

    size_t n = c->hashfun(k);

    hash_rehash_step(c);

    struct hash_item *e = hash_item_create(c, k, v);

    if( !e ) {
        return false;
    }

    hash_table_add(to_add(c), n, e);

    hash_rehash_start(c);

    return true;
}

static inline struct hash_item *hash_table_get(struct hash_table *t, size_t n) {
    return t->data[bucket(t, n)];
}

static inline void hash_table_update(struct hash_table *t, size_t n, struct hash_item *e) {
    size_t i = bucket(t, n);
    t->data[i] = e;
    if( !t->data[i] && t->used ) {
        t->used--;
    }
}

static inline void hash_find_all( struct hash *c
                                , void *k
                                , void *cc
                                , bool (*fn)(void*,struct hash_item*)) {

    uint32_t n = c->hashfun(k);

    size_t col = 0;

    size_t tn = 0;

    for( ;tn < 2; tn++ ) {

        if( !c->table[tn] ) {
            continue;
        }

        struct hash_item *e = hash_table_get(c->table[tn], n);

        while(e) {
            void *kk = hash_item_key(c,e);
            if( c->hashfun(kk) == n && c->keycmp(kk, k)  ) {
                if(!fn(cc, e) ) {
                    tn = 2;
                    break;
                }
            }
            col++; // number of collisions
            e = e->next;
        }
    }

    // TODO:
    if( 0 ) { // wtf ?
        // begin rehashing
    }
}

static bool __hash_find_first(void *cc, struct hash_item *it) {
    *(struct hash_item**)cc = it;
    return false;
}

void *hash_get(struct hash *c, void *k) {

    struct hash_item *it = 0;
    hash_find_all(c, k, &it, __hash_find_first);
    return it ? hash_item_val(c, it) : 0;
}

struct hash_find_cc {
    struct hash *hash;
    void *cc;
    void (*cb)(void*,void*);
};

static bool __hash_find_every(void *cc_, struct hash_item *e) {
    struct hash_find_cc *cc = cc_;
    cc->cb(cc->cc, hash_item_val(cc->hash, e));
    return true;
}

void hash_find( struct hash *c
              , void *k
              , void *cc
              , void (*cb) (void *cc, void *v)) {


    struct hash_find_cc fcc = { .hash = c, .cc = cc, cb = cb };
    hash_find_all(c, k, &fcc, __hash_find_every);
}

struct hash_alter_cc {
    struct hash *hash;
    void *cc;
    void (*fn) (void *, void *, void *, bool);
    size_t n;
};

static bool __hash_alter_every(void *cc, struct hash_item *e) {

    struct hash_alter_cc *acc = cc;

    acc->fn( acc->cc
           , hash_item_key(acc->hash, e)
           , hash_item_val(acc->hash, e)
           , false);

    acc->n++;

    return true;
}

bool hash_alter( struct hash* c
               , bool add
               , void *k
               , void *cc
               , void (*fn) (void *, void *, void *, bool)) {

    hash_rehash_step(c);

    struct hash_alter_cc acc = { .hash = c, .cc = cc, fn = fn };
    hash_find_all(c, k, &acc, __hash_alter_every);

    if( !acc.n && add ) {

        size_t n = c->hashfun(k);
        struct hash_item *e = hash_item_create(c, k, 0);

        if( !e ) {
            return false;
        }

        hash_table_add(to_add(c), n, e);
        fn(cc, hash_item_key(c, e), hash_item_val(c, e), true);
        acc.n++;
    }

    return acc.n != 0;
}

void hash_del(struct hash *c, void *k) {
    uint32_t n = c->hashfun(k);
    size_t tn = 0;
    for(; tn < 2; tn++ ) {

        if( !c->table[tn] ) {
            continue;
        }

        struct hash_item *e = hash_table_get(c->table[tn], n);
        struct hash_item *ne = 0;

        while(e) {
            struct hash_item *it = e;
            e = e->next;

            void *kk = hash_item_key(c, it);
            uint32_t hc = c->hashfun(kk);

            if( n == hc && c->keycmp(kk, k) ) {
                c->dealloc(c->allocator, it);
            } else {
                it->next = ne;
                ne = it;
            }
        }

        hash_table_update(c->table[tn], n, ne);
    }
}

void hash_filter( struct hash *c
                , void *cc
                , bool (*cb)(void *, void *, void *)) {

    size_t tn = 0;
    for(; tn < 2; tn++ ) {

        struct hash_table *t = c->table[tn];

        if( !t ) {
            continue;
        }

        size_t i = 0;
        for(; i < t->capacity; i++ ) {
            struct hash_item *e = t->data[i];
            struct hash_item *ne = 0;
            while(e) {
                struct hash_item *it = e;
                e = e->next;
                void *k = hash_item_key(c, it);
                void *v = hash_item_val(c, it);
                if( cb && cb(cc, k, v) ) {
                    it->next = ne;
                    ne = it;
                } else {
                    c->dealloc(c->allocator, it);
                }
            }
            t->data[i] = ne;
        }
    }
}

void hash_enum( struct hash *c
              , void *cc
              , void (*cb)(void *, void *, void *)) {


    size_t tn = 0;
    for(; tn < 2; tn++ ) {

        struct hash_table *t = c->table[tn];

        if( !t ) {
            continue;
        }

        size_t i = 0;
        for(; i < t->capacity; i++ ) {
            struct hash_item *it =  t->data[i];
            for(; it; it = it->next ) {
                cb(cc, hash_item_key(c, it), hash_item_val(c, it));
            }
        }
    }

}

void hash_stats( struct hash *c
               , size_t *capacity
               , size_t *used
               , size_t *collisions
               , size_t *maxbuck
               ) {

    *capacity = c->active->capacity;
    *used = c->active->used;

    size_t i = 0;
    size_t total = 0;

    for(; i < c->active->capacity; i++ ) {
        struct hash_item *e = c->active->data[i];
        size_t row = 0;
        for(; e; e = e->next, row++ );
        *maxbuck = row > *maxbuck ? row : *maxbuck;
        total += row ? row - 1 : 0;
    }

    *collisions = *used ? total / *used : 0;
}


static size_t hash_item_size(struct hash *c) {
    return sizeof(struct hash_item) + c->keysize + c->valsize;
}

static inline void *hash_item_key(struct hash *c, struct hash_item *e) {
    return &e->data[0];
}

static inline void *hash_item_val(struct hash *c, struct hash_item *e) {
    return &e->data[c->keysize];
}

static inline size_t hash_table_size(size_t n) {
    return sizeof(struct hash_table) + n*sizeof(struct hash_table*);
}

static inline bool hash_alloc_table(struct hash *c, size_t n, size_t buck) {

    void *mem = c->alloc(c->allocator, hash_table_size(buck));

    if( mem ) {
        struct hash_table *tbl = mem;
        tbl->capacity = buck;
        tbl->used = 0;
        memset(tbl->data, 0, buck*sizeof(void*));
        c->table[n] = tbl;
        return true;

    }

    return false;
}

static inline void hash_set_active(struct hash *c, size_t n) {
    c->active = c->table[n];
}


static inline void hash_set_shadow(struct hash *c, size_t n) {
    c->shadow = c->table[n];
}

