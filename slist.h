#ifndef __slist_h
#define __slist_h

#include <stddef.h>

typedef struct slist_ {
    struct slist_ *next;
    char value[0];
} slist;

size_t slist_size(size_t psize); 
slist* slist_cons(slist *x, slist *xs);
slist* slist_uncons(slist **xs); 
slist* slist_pool(void *mem, size_t chunk_size, size_t size); 
slist* slist_alloc(slist **pool, void *cc, void (*init)(void*, char*));
slist *const slist_nil();

#endif

