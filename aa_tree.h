#ifndef __aa_tree_h
#define __aa_tree_h

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

struct aa_tree;
extern const size_t aa_tree_size;


struct aa_tree *aa_tree_create( size_t memsize
                              , void *mem
                              , size_t itemsize
                              , bool (*less)(void*,void*)
                              , bool (*eq)(void*,void*)
                              , void (*cpy)(void*,void*)
                              , void *allocator
                              , void *(*alloc)(void*,size_t)
                              , void  (*dealloc)(void*,void*) );

void aa_tree_destroy(struct aa_tree *t);
bool aa_tree_insert(struct aa_tree *t, void *v);
void aa_tree_remove(struct aa_tree *t, void *v);
void *aa_tree_find( struct aa_tree *t, void *v );


// for debugging
struct aa_node_info {
    size_t level;
    void  *value;
};

void aa_tree_enum_debug( struct aa_tree *t
                       , void *cc_
                       , void (*fn)( void*
                                   , struct aa_node_info *p
                                   , struct aa_node_info *n ) );

#endif

