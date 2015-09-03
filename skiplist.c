#include "skiplist.h"
#include "slist.h"

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

struct skipnode;

struct skiplist {

    struct skipnode *top;
    struct skipnode *bottom;

    size_t  itemsize;
    uint8_t maxlevel;
    uint8_t level;

    bool (*leq)(void*, void*);
    void (*cpy)(void*, void*);

    void *allocator;
    void *(*alloc)(void*,size_t);
    void (*dealloc)(void*,void*);

    void *rndgen;
    uint64_t (*rnd)(void*);
};

const size_t skiplist_size = sizeof(struct skiplist);

static uint64_t __minus_inf_value = 10101010;
static uint64_t __plus_inf_value  = 11111111;

static void *const __minus_inf = &__minus_inf_value;
static void *const __plus_inf  = &__plus_inf_value;

struct skipnode {
    struct skipnode *next;
    struct skipnode *down;
    void *value;
    char data[0];
};

static void skiplist_add_levels(struct skiplist *sl, uint8_t n);

static struct skipnode* skipnode_init( struct skiplist*
                                     , bool
                                     , struct skipnode *
                                     , void *);

struct skiplist * skiplist_create( size_t memsize
                                 , void *mem
                                 , size_t maxlevel
                                 , size_t itemsize
                                 , bool (*leq)(void*,void*)
                                 , void (*cpy)(void*,void*)
                                 , void *allocator
                                 , void *(*alloc)(void*,size_t)
                                 , void  (*dealloc)(void*,void*)
                                 , void *rndgen
                                 , uint64_t (*rnd)(void*) ) {

    if( memsize < skiplist_size ) {
        return 0;
    }

    struct skiplist *sl = mem;

    sl->top = 0;
    sl->bottom = 0;
    sl->level = 0;
    sl->maxlevel = maxlevel;
    sl->itemsize = itemsize;
    sl->leq = leq;
    sl->cpy = cpy;
    sl->allocator = allocator;
    sl->alloc = alloc;
    sl->dealloc = dealloc;
    sl->rndgen = rndgen;
    sl->rnd = rnd;

    skiplist_add_levels(sl, 1);

    return sl;
}

void skiplist_destroy(struct skiplist *sl) {
    while( sl->top ) {
        while( sl->top->next ) {
            struct skipnode *n = sl->top->next;
            sl->top->next = n->next;
            sl->dealloc(sl->allocator, n);
        }
        struct skipnode *n = sl->top;
        sl->top = n->down;
        sl->dealloc(sl->allocator, n);
    }
}

static struct skipnode *skipnode_init( struct skiplist *sl
                                     , bool cpy
                                     , struct skipnode *n
                                     , void *v ) {

    if( !n ) {
        return 0;
    }

    n->next = 0;
    n->down = 0;

    if( !cpy ) {
        n->value = v;
    } else {
        sl->cpy(n->data, v);
        n->value = n->data;
    }
    return n;
}

static struct skipnode *skipnode_insert_next(struct skipnode *p, struct skipnode *n) {
    if( p && n ) {
        n->next = p->next;
        p->next = n;
        return n;
    }
    return 0;
}

static inline void adjust_bottom(struct skiplist *sl) {
    if( !sl->bottom ) {
        sl->bottom = sl->top;
    }
    for(; sl->bottom->down; sl->bottom = sl->bottom->down);
}

static
struct skipnode * skipnode_insert_up( struct skipnode **t
                                    , struct skipnode *n) {

    if( n ) {
        n->down = *t;
        *t = n;
        return n->down;
    }

    return 0;
}

static bool skipnode_value_leq( struct skiplist *sl
                              , void *a
                              , void *b ) {


    if( a == __minus_inf ) {
        return true;
    }

    if( b == __minus_inf ) {
        return false;
    }

    if( b == __plus_inf ) {
        return false;
    }

    if( a == __plus_inf ) {
        return false;
    }

    return sl->leq(a,b);
}

static inline bool __safe_eq( bool (*eq)(void*,void*), void *a, void *b ) {

    if( a == __minus_inf || a == __plus_inf )
        return false;

    if( b == __minus_inf || b == __plus_inf )
        return false;

    return eq(a,b);
}

static void skipnode_find( struct skiplist *sl
                         , struct skipnode *from
                         , void *v
                         , void *cc
                         , bool (*fn)( void*
                                     , struct skipnode*
                                     , struct skipnode*) ) {

    struct skipnode *p = from;

    for(; p; ) {
        struct skipnode *prev = 0;
        for(; skipnode_value_leq(sl, p->next->value, v); p = p->next) {
            prev = p;
        }
        if( !fn(cc, prev, p) ) {
            return;
        }
        p = p->down;
    }
}

static uint8_t skiplist_new_level(struct skiplist *sl) {

    uint8_t lvl = 1;

    while( lvl <= sl->maxlevel && (sl->rnd(sl->rndgen) & 0xFFFF) < (0xFFFF/2) ) {
        lvl++;
    }

    return (uint8_t)MIN(lvl, sl->maxlevel);
}

static struct skipnode * skipnode_create(struct skiplist *sl, void *v, size_t sz) {
    return skipnode_init( sl
                        , !!sz
                        , sl->alloc(sl->allocator, (sizeof(struct skipnode) + sz))
                        , v );
}

static void skiplist_add_levels(struct skiplist *sl, uint8_t n) {

    for(; n; n--, sl->level++ ) {
        skipnode_insert_up( &sl->top
                          , skipnode_create(sl, __minus_inf, 0));

        skipnode_insert_next(sl->top, skipnode_create(sl, __plus_inf, 0));
    }

    adjust_bottom(sl);
}

struct skipnode_path_cc {
    slist *free;
    slist *path;
};

static bool __skiplist_mem_path( void *cc_
                               , struct skipnode *prev
                               , struct skipnode *n) {

    struct skipnode_path_cc *cc = cc_;
    slist *e = slist_uncons(&cc->free);

    if( e ) {
        struct skipnode **v = slist_value(struct skipnode**, e);
        *v = n;
        cc->path = slist_cons(e, cc->path);
    } else {
        // FIXME: error handling!
        return false;
    }

    return true;
}

static inline bool __allocate_path(struct skiplist *sl, slist **f, void **p) {
    const size_t itemsz = slist_size(sizeof(struct skipnode*));
    const size_t poolsz = sl->maxlevel*itemsz;

    *p = sl->alloc(sl->allocator, poolsz);

    if( ! (*p) ) {
        return false;
    }

    *f = slist_pool(*p, itemsz, poolsz);

    return !!(*f);
}

static inline void __deallocate_path(struct skiplist *sl, void* pool) {
    sl->dealloc(sl->allocator, pool);
}

bool skiplist_insert(struct skiplist *sl, void *v) {

    uint8_t lvl = skiplist_new_level(sl);

    if( lvl > sl->level ) {
        skiplist_add_levels(sl, lvl - sl->level);
    }

    void *pool = 0;
    slist *free = 0;

    if( !__allocate_path(sl, &free, &pool) ) {
        return false;
    }

    struct skipnode_path_cc cc = { .free = free
                                 , .path = slist_nil()
                                 };

    skipnode_find( sl
                 , sl->top
                 , v
                 , &cc
                 , __skiplist_mem_path );


    if( !cc.path ) {
        // FIXME: error!
        return false;
    }

    struct skipnode** n = slist_value(struct skipnode**, slist_uncons(&cc.path));

    struct skipnode *nn = 0;
    nn = skipnode_insert_next(*n, skipnode_create(sl, v, sl->itemsize));
    lvl--;

    slist *e = 0;
    struct skipnode *down = nn;

    for(; lvl && (e = slist_uncons(&cc.path)); lvl--) {
        struct skipnode **p = slist_value(struct skipnode**, e);
        struct skipnode *q = skipnode_insert_next(*p, skipnode_create(sl, down->value, 0));
        q->down = down;
        down = q;
    }

    __deallocate_path(sl, pool);

    return true;
}

struct find_match_cc {
    struct skipnode *n;
    void *v;
    bool (*eq)(void*,void*);
};

static bool __find_match(void *cc_, struct skipnode *p, struct skipnode *n) {

    struct find_match_cc *cc = cc_;

    if( !n || n->value == __minus_inf ) {
        return true;
    }

    if( __safe_eq(cc->eq, n->value, cc->v) ) {
        cc->n = n;
        return false;
    }

    // should not happen
    return true;
}

void *skiplist_find( struct skiplist *sl
                   , void *v
                   , bool (*eq)(void*,void*)
                   ) {

    struct find_match_cc cc = { .n  = 0
                              , .v  = v
                              , .eq = eq
                              };

    skipnode_find(sl, sl->top, v, &cc, __find_match);

    return cc.n ? cc.n->value : 0;
}


struct skipnode_rm_cc {
    void  *v;
    bool  (*eq)(void*,void*);
    struct skipnode *p;
};

static bool __skiplist_rm_fn( void *cc_
                            , struct skipnode *prev
                            , struct skipnode *n) {

    struct skipnode_rm_cc *cc = cc_;

    if( n && !__safe_eq(cc->eq, cc->v, n->value) ) {
        cc->p = n;
        return false;
    }

    if( prev ) {
        cc->p = prev;
        return false;
    }

    return true;
}

static inline bool empty_level( struct skipnode *n ) {
    return n->value == __minus_inf && n->next->value == __plus_inf;
}

static void prune_levels(struct skiplist *sl) {

    struct skipnode *p    = sl->top;

    struct skipnode *ntop = 0;

    while( p ) {
        struct skipnode *c = p;
        p = p->down;

        if( empty_level(c) ) {
            while( c ) {
                struct skipnode *tmp = c->next;
                sl->dealloc(sl->allocator, c);
                c = tmp;
            }
        } else {
            c->down = ntop;
            ntop = c;
        }
    }

    sl->bottom = 0;
    sl->top = 0;
    sl->level = 0;

    while( ntop ) {
        struct skipnode *c = ntop;
        ntop = ntop->down;
        c->down = sl->top;
        sl->top = c;
        sl->level++;
    }

    adjust_bottom(sl);
}

bool skiplist_remove( struct skiplist *sl
                    , void *v
                    , bool (*eq)(void*,void*)
                    ) {

    size_t rm = 0;

    struct skipnode_rm_cc cc = { .p = 0, .v = v, .eq = eq } ;

    struct skipnode *p = sl->top;

    for(; p; ) {

        skipnode_find( sl
                     , p
                     , v
                     , &cc
                     , __skiplist_rm_fn );


        if( cc.p->next && __safe_eq(eq, cc.p->next->value, v) ) {
            struct skipnode *tmp = cc.p->next;
            cc.p->next = cc.p->next->next;
            sl->dealloc(sl->allocator, tmp);
            rm++;
        }

        p = cc.p ? cc.p->down : 0;
    }

   prune_levels(sl);

    return (rm > 0);
}

void skiplist_enum( struct skiplist *sl
                  , void *cc
                  , void (*fn)(void*,void*) ) {

    // should not never ever been nil
    struct skipnode *p = sl->bottom->next;
    for(; p && p->value != __plus_inf; p = p->next) {
        fn(cc, p->value);
    }
}

static int __value_tp( void *n ) {
    if( n == __minus_inf )
        return -1;

    if( n == __plus_inf )
        return 1;

    return 0;
}

void skiplist_enum_debug( struct skiplist *sl
                        , void *cc
                        , void (*fn)( void *cc
                                    , uint8_t level
                                    , int tp
                                    , void *v ) ) {

    uint8_t lvl = sl->level;
    struct skipnode *tp = sl->top;
    for(; tp; tp = tp->down, lvl-- ) {
        struct skipnode *p = tp;
        for(; p; p = p->next ) {
            int ttp = __value_tp(p->value);
            fn(cc, lvl, ttp, !ttp ? p->value : 0);
        }
    }

}

