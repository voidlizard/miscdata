#include "aa_tree.h"

typedef enum { L = 0, R = 1 } lr;

struct aa_node;

struct aa_tree {
    struct aa_node *root;

    size_t itemsize;

    bool (*less)(void*,void*);
    bool (*eq)(void*,void*);
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
                              , bool (*less)(void*,void*)
                              , bool (*eq)(void*,void*)
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
    t->less = less;
    t->eq = eq;
    t->cpy = cpy;
    t->allocator = allocator;
    t->alloc = alloc;
    t->dealloc = dealloc;

    return t;
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

static struct aa_node *aa_node_create( struct aa_tree *t, void *v ) {
    struct aa_node *n = t->alloc(t->allocator, aa_node_size + t->itemsize);

    if( !n ) {
        return 0;
    }

    return aa_node_init(t, n, v);
}

static inline lr dir_of(bool le) {
    return le ? L : R;
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

static struct aa_node *aa_node_insert( struct aa_tree *t
                                     , struct aa_node *to
                                     , void *v ) {

    if( to == aa_node_null ) {
        // create new leaf
        return aa_node_create(t, v);
    }

    if( t->eq( aa_value(to), v ) ) {
        return to;
    }

    size_t dir = dir_of(t->less(v, aa_value(to)));

    to->child[dir] = aa_node_insert(t, to->child[dir], v);

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

    lr dir = dir_of(t->less(v, aa_value(n)));

    if( dir == R ) {
        *deleted = n;
    }

    n->child[dir] = aa_node_remove(t, n->child[dir], v, last, deleted);

    if( n == *last && !aa_nil(*deleted) && t->eq(v, aa_value(*deleted) ) ) {
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

    if( !n || aa_node_null == n ) {
        return 0;
    }

    if( t->eq( aa_value(n), v ) ) {
        return n;
    }

    *p = n;
    return aa_node_find(t,v,n->child[dir_of(t->less(v,aa_value(n)))],p);
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


bool aa_tree_insert(struct aa_tree *t, void *v) {
    struct aa_node *n = aa_node_insert(t, t->root, v);

    if( !n ) {
        return false;
    }

    t->root = n;
    return true;
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


