#include <assert.h>
#include "slist.h"

static slist *__slist_nil = 0;

slist *const slist_nil() {
    return __slist_nil;
}

slist* slist_cons(slist *x, slist *xs) {
    assert(x);
    x->next = xs;
    return x;
}

slist* slist_uncons(slist **xs) {
    slist *tmp = *xs;
    if( !(*xs) ) return 0;
    *xs = (*xs)->next;
    return tmp;
}

slist* slist_pool(void *mem, size_t chunk_size, size_t size) {
    size_t chunks = size / chunk_size;
    char *p  = (char*)mem;
    char *pe = (char*)mem + chunks*chunk_size;

    slist *pool = slist_nil();
    for(; p < pe; p += chunk_size ) {
        pool = slist_cons((slist*)p, pool);
    }

    return pool;
}

slist* slist_alloc(slist **pool, void *cc, void (*init)(void*, char*)) {
    slist *tmp = slist_uncons(pool);
    if(!tmp) return 0;
    tmp->next = (slist*)slist_nil();
    if(init) init(cc, tmp->value);
    return tmp;
}

