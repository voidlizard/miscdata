#include "aa_map.h"

#define safecall(v, f, ...) ((f) ? (f(__VA_ARGS__)) : (v))
#define unit                {}

typedef enum { L = 0, R = 1 } lr;

struct aa_node;

struct aa_tree {
    struct aa_node *root;

    size_t itemsize;

    int (*cmp)(void*,void*);
    void (*cpy)(void*,void*);

    void *allocator;
    void *(*alloc)(void*,size_t);
    void  (*dealloc)(void*,void*);
};

const size_t aa_tree_size = sizeof(struct aa_tree);

struct aa_node {
    size_t level;
    struct aa_node *child[2];
    uint8_t data[0];
};

static struct aa_node aa_node_bottom = { .level = 0
                                       , .child = { &aa_node_bottom
                                                  , &aa_node_bottom
                                                  }
                                       };

static struct aa_node *aa_node_null = &aa_node_bottom;

const size_t aa_node_size = sizeof(struct aa_node);

static inline void *aa_value( struct aa_node *n ) {
    return (void*)n->data;
}

static inline struct aa_node* l(struct aa_node *n) {
    return n->child[L];
}

static inline struct aa_node* r(struct aa_node *n) {
    return n->child[R];
}

static inline bool aa_nil( struct aa_node *n ) {
    return !n || n == aa_node_null;
}

static inline bool aa_leaf( struct aa_node *n ) {
    return aa_nil(l(n)) && aa_nil(r(n));
}

struct aa_tree *aa_tree_create( size_t memsize
                              , void *mem
                              , size_t itemsize
                              , int (*cmp)(void*,void*)
                              , void (*cpy)(void*,void*)
                              , void *allocator
                              , void *(*alloc)(void*,size_t)
                              , void  (*dealloc)(void*,void*) ) {


    if( memsize < aa_tree_size ) {
        return 0;
    }

    struct aa_tree *t = mem;
    t->root = aa_node_null;
    t->itemsize = itemsize;
    t->cmp = cmp;
    t->cpy = cpy;
    t->allocator = allocator;
    t->alloc = alloc;
    t->dealloc = dealloc;

    return t;
}

static inline size_t dir_of(int cmp) {
    return cmp < 0 ? L : R;
}

static void aa_node_destroy( struct aa_tree *t, struct aa_node *v ) {
    if( aa_nil(v) ) {
        return;
    }
    aa_node_destroy(t, l(v));
    aa_node_destroy(t, r(v));
    t->dealloc(t->allocator, v);
}

static inline struct aa_node* aa_skew(struct aa_node *n) {

    if( aa_nil(n) ) {
        return n;
    }

    if( n->child[L]->level == n->level && n->level ) {
        struct aa_node *tmp = n->child[L];
        n->child[L] = tmp->child[R];
        tmp->child[R] = n;
        n = tmp;
    }

    return n;
}

static inline struct aa_node* aa_split(struct aa_node *n)
{
    if( aa_nil(n) ) {
        return n;
    }

    if(n->child[R]->child[R]->level == n->level && n->level != 0) {
        struct aa_node *tmp = n->child[R];

        n->child[R] = tmp->child[L];
        tmp->child[L] = n;
        n = tmp;
        n->level++;
    }

    return n;
}

static struct aa_node *aa_node_init( struct aa_tree *t
                                   , struct aa_node *n
                                   , void *v) {

    if( !n ) {
        return n;
    }

    *n = aa_node_bottom;
    n->level = 1;
    n->child[L] = n->child[R] = aa_node_null;
    t->cpy(&n->data[0], v);

    return n;
}

static struct aa_node *aa_node_create(struct aa_tree *t, void *v) {
    struct aa_node *n = t->alloc(t->allocator, aa_node_size + t->itemsize);

    if( !n ) {
        return 0;
    }

    return aa_node_init(t, n, v);
}

static struct aa_node *aa_node_insert( struct aa_tree *t
                                     , struct aa_node *to
                                     , void *v
                                     , void *cc
                                     , void (*fn)(void*,void*,bool)
                                     ) {

    if( to == aa_node_null ) {
        struct aa_node *n = aa_node_create(t, v);
        safecall(unit, fn, cc, aa_value(n), true);
        return n;
    }

    int cmp = t->cmp(v, aa_value(to));

    if( !cmp ) {
        safecall(unit, fn, cc, aa_value(to), false);
        return to;
    }

    const lr dir = dir_of(cmp);

    to->child[dir] = aa_node_insert(t, to->child[dir], v, cc, fn);

    return aa_split(aa_skew(to));
}

static struct aa_node* aa_node_remove( struct aa_tree *t
                                     , struct aa_node *n
                                     , void *v
                                     , struct aa_node **last
                                     , struct aa_node **deleted
                                     ) {

    if( aa_nil(n) ) {
        return n;
    }

    *last = n;

    int cmp = t->cmp(v, aa_value(n));

    lr dir = dir_of(cmp);

    if( dir == R ) {
        *deleted = n;
    }

    n->child[dir] = aa_node_remove(t, n->child[dir], v, last, deleted);

    if( n == *last && !aa_nil(*deleted) && !t->cmp(v, aa_value(*deleted) ) ) {
        // we're about to delete the value??
        t->cpy((*deleted)->data, n->data);
        *deleted = aa_node_null;
        n = r(n);
        t->dealloc(t->allocator, (*last));
    } else if( l(n)->level < n->level-1 || r(n)->level < n->level-1)  {
        //go up a level and rebalance?
        n->level--;
        if( r(n)->level > n->level ) {
            r(n)->level = n->level;
        }
        n = aa_skew(n);
        n->child[R] = aa_skew( r(n) );
        n->child[R]->child[R] = aa_skew( r(r(n)) );
        n = aa_split(n);
        n->child[R] = aa_split(r(n));
    }

    return n;
}


static void aa_node_walk( struct aa_node *p
                        , struct aa_node *n
                        , void *cc
                        , void (*fn)( void*
                                    , struct aa_node *
                                    , struct aa_node *)) {

    if( aa_nil(n) ) {
        return;
    }

    aa_node_walk(n, n->child[L], cc, fn);
    fn(cc, p, n);
    aa_node_walk(n, n->child[R], cc, fn);

}

static struct aa_node *aa_node_find( struct aa_tree *t
                                   , void *v
                                   , struct aa_node *n
                                   , struct aa_node **p ) {

    if( aa_nil(n) ) {
        return 0;
    }

    if( !t->cmp( aa_value(n), v ) ) {
        return n;
    }

    *p = n;
    return aa_node_find(t,v,n->child[dir_of(t->cmp(v,aa_value(n)))],p);
}

void *aa_tree_find( struct aa_tree *t, void *v ) {
    struct aa_node *p = 0;
    struct aa_node *n = aa_node_find(t, v, t->root, &p);
    return n ? aa_value(n) : 0;
}

void aa_tree_remove(struct aa_tree *t, void *v) {
    struct aa_node *l = 0, *d = 0;
    t->root = aa_node_remove(t, t->root, v, &l, &d);
}

static bool aa_tree_alter (struct aa_tree *t
                         , void *v
                         , void *cc
                         , void (*fn)(void*,void*,bool)) {


    struct aa_node *n = aa_node_insert(t, t->root, v, cc, fn);

    if( !n ) {
        return false;
    }

    t->root = n;
    return true;
}

bool aa_tree_insert(struct aa_tree *t, void *v) {
    return aa_tree_alter(t,v,0,0);
}

void aa_tree_destroy(struct aa_tree *t) {
    aa_node_destroy(t, t->root);
    t->root = aa_node_null;
}

struct aa_tree_enum_cc {
    void *cc;
    void (*fn)(void*,void*);
};

static void __aa_tree_walk_fn0( void *cc_
                              , struct aa_node *p
                              , struct aa_node *n) {

    struct aa_tree_enum_cc *cc = cc_;
    cc->fn(cc->cc, aa_value(n));
}

void aa_tree_enum( struct aa_tree *t
                 , void *cc_
                 , void (*fn)(void*, void*)) {

    struct aa_tree_enum_cc cc = { .cc = cc_
                                , .fn = fn
                                };

    aa_node_walk(aa_node_null, t->root, &cc, __aa_tree_walk_fn0);
}


struct aa_tree_walk_debug_cc {
    void *cc;
    void (*fn)(void*,struct aa_node_info*,struct aa_node_info*);
};

static void __aa_tree_walk_debug_fn( void *cc_
                                   , struct aa_node *p
                                   , struct aa_node *n) {

    struct aa_tree_walk_debug_cc *cc = cc_;

    struct aa_node_info prev = { .level = !aa_nil(p) ? p->level : 0
                               , .value = !aa_nil(p) ? aa_value(p) : 0
                               };

    struct aa_node_info curr = { .level = !aa_nil(n) ? n->level : 0
                               , .value = !aa_nil(n) ? aa_value(n) : 0
                               };

    cc->fn(cc->cc, &prev, &curr);
}


void aa_tree_enum_debug( struct aa_tree *t
                       , void *cc_
                       , void (*fn)( void*
                                   , struct aa_node_info *p
                                   , struct aa_node_info *n ) ) {

    struct aa_tree_walk_debug_cc cc = { .cc = cc_
                                      , .fn = fn
                                      };

    aa_node_walk(aa_node_null, t->root, &cc, __aa_tree_walk_debug_fn);
}

// aa map

struct aa_map {

    size_t keysize;
    size_t valsize;

    void (*keycpy)(void*,void*);
    void (*valcpy)(void*,void*);
    int  (*keycmp)(void*,void*);

    struct aa_tree *t;
    char aa_tree_mem[sizeof(struct aa_tree)];
};

struct aa_map_cell {
    struct aa_map *m;
    char data[0];
};

const size_t aa_map_size = sizeof(struct aa_map);

static inline size_t aa_map_cell_size(size_t k, size_t v) {
    return (sizeof(struct aa_map_cell) + k + v);
}

static inline void *aa_cell_key(struct aa_map_cell *c) {
    return &c->data[0];
}

static inline void *aa_cell_val(struct aa_map_cell *c) {
    return &c->data[c->m->keysize];
}

static void aa_map_cell_cpy(void *a_, void *b_) {
    struct aa_map_cell *a = a_;
    struct aa_map_cell *b = b_;
    struct aa_map *m = b->m;

    a->m = m;
    m->keycpy(aa_cell_key(a), aa_cell_key(b));

    if( aa_cell_val(b) ) {
        m->valcpy(aa_cell_val(a), aa_cell_val(b));
    }
}

static int aa_map_cell_cmp(void *a_, void *b_) {
    struct aa_map_cell *a = a_;
    struct aa_map_cell *b = b_;
    return a->m->keycmp(aa_cell_key(a), aa_cell_key(b));
}

struct aa_map *aa_map_create( size_t memsize
                            , void *mem
                            , size_t keysize
                            , size_t valsize
                            , int cmp(void*,void*) // compare KEYS!
                            , void (*keycpy)(void*,void*)
                            , void (*valcpy)(void*,void*)
                            , void *allocator
                            , void *(*alloc)(void*,size_t)
                            , void  (*dealloc)(void*,void*) ) {


    if(memsize < aa_map_size) {
        return 0;
    }

    struct aa_map *m = mem;
    m->keysize = keysize;
    m->valsize = valsize;
    m->keycpy = keycpy;
    m->valcpy = valcpy;
    m->keycmp = cmp;

    m->t = aa_tree_create( sizeof(m->aa_tree_mem)
                         , m->aa_tree_mem
                         , aa_map_cell_size(keysize, valsize)
                         , aa_map_cell_cmp
                         , aa_map_cell_cpy
                         , allocator
                         , alloc
                         , dealloc );


    if( !m->t ) {
        return 0;
    }

    return m;
}

static inline struct aa_map_cell *aa_map_cell_init( struct aa_map *m
                                                  , struct aa_map_cell *c
                                                  , void *k
                                                  , void *v) {

    c->m = m;
    m->keycpy(aa_cell_key(c), k);
    if( v ) {
        m->valcpy(aa_cell_val(c), v);
    }
    return c;
}

bool aa_map_add(struct aa_map *m, void *k, void *v) {
    struct aa_map_cell cell;
    return aa_tree_insert(m->t, aa_map_cell_init(m, &cell, k, v));
}

void *aa_map_find(struct aa_map *m, void *k) {
    struct aa_map_cell cell;
    struct aa_map_cell *r = aa_tree_find( m->t
                                        , aa_map_cell_init(m, &cell, k, 0));
    return r ? aa_cell_val(r) : 0;
}

void aa_map_del(struct aa_map *m, void *k) {
    struct aa_map_cell cell;
    aa_tree_remove( m->t
                  , aa_map_cell_init(m, &cell, k, 0));
}

struct aa_enum_fn_cc {
    void *cc;
    void (*fn)(void*,void*,void*);
};

static void __aa_enum_fn(void *cc_, void *v) {
    struct aa_enum_fn_cc *cc = cc_;
    cc->fn(cc->cc, aa_cell_key(v), aa_cell_val(v));
}

void aa_map_enum(struct aa_map *m, void *cc, void (*fn)(void*,void*,void*)) {
    struct aa_enum_fn_cc rcc = { .cc = cc,  .fn = fn };
    aa_tree_enum(m->t, &rcc, __aa_enum_fn);
}

void aa_map_destroy(struct aa_map *m) {
    aa_tree_destroy(m->t);
}

struct aa_map_alter_cc {
    void *cc;
    void (*fn)(void*,void*,void*,bool);
};

static void aa_map_alter_fn(void *cc, void *v, bool n) {
    struct aa_map_alter_cc *acc = cc;
    acc->fn(acc->cc, aa_cell_key(v), aa_cell_val(v), n);
}

bool aa_map_alter( struct aa_map *m
                 , bool create
                 , void *k
                 , void *cc
                 , void (*fn)( void*   // cc
                             , void*   // k
                             , void*   // v
                             , bool)) {

    struct aa_map_alter_cc acc = { .cc = cc, .fn = fn };

    struct aa_map_cell cell;
    struct aa_map_cell *cp = aa_map_cell_init(m, &cell, k, 0);

    if( create ) {
        return aa_tree_alter(m->t, cp, &acc, aa_map_alter_fn);
    } else {
        struct aa_node *p = 0;
        struct aa_node *n = aa_node_find(m->t, cp, m->t->root, &p);
        if( n ) {
            struct aa_map_cell *cp = aa_value(n);
            fn(cc, aa_cell_key(cp), aa_cell_val(cp), false);
            return true;
        }
    }

    return false;
}

static void aa_node_unlink(struct aa_node *c, struct aa_node **ns) {

    if( aa_nil(c) ) {
        return;
    }

    struct aa_node *lp = l(c);
    struct aa_node *rp = r(c);

    c->child[L] = 0;
    c->child[R] = *ns;
    (*ns) = c;

    aa_node_unlink(lp, ns);
    aa_node_unlink(rp, ns);
}

void aa_map_filter(struct aa_map *m, void *cc, bool (*fn)(void*,void*,void*)) {
    struct aa_node *ns = 0;
    aa_node_unlink(m->t->root, &ns);

    struct aa_tree *t = m->t;
    t->root = aa_node_null;

    while(ns) {
        void *cell = aa_value(ns);
        void *k = aa_cell_key(cell);
        void *v = aa_cell_val(cell);

        if( fn && fn(cc, k, v) ) {
            aa_map_add(m, k, v);
        }

        void *zombie = ns;
        ns = r(ns);
        t->dealloc(t->allocator, zombie);
    }
}

