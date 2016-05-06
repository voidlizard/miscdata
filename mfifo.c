
#include "mfifo.h"
#include "miscdata.h"

struct chunk {
    struct chunk *n; // __MUST__ be the first element of the chunk
    struct chunk *p;
    char data[0];
};

struct mfifo {
    size_t chunk_size;
    size_t free_chunks_max;
    size_t free_chunks_num;

    struct chunk *head;
    struct chunk *tail;
    struct chunk *free;

    // allocator
    void *allocator;
    void *(*alloc)(void *, size_t);
    void (*dealloc)(void*,void*);
};

const size_t mfifo_size(void) {
    return sizeof(struct mfifo);
}

struct mfifo *mfifo_create( void   *mem
                                        , size_t memsize
                                        , size_t chunk_size
                                        , size_t free_chunks_max
                                        , void *allocator
                                        , void *(*alloc)(void *, size_t)
                                        , void (*dealloc)(void*,void*)
                                        ) {


    if( memsize < mfifo_size() ) {
        return 0;
    }

    struct mfifo *fifo = mem;

    fifo->chunk_size = chunk_size;
    fifo->free_chunks_max = free_chunks_max;
    fifo->free_chunks_num = 0;
    fifo->allocator = allocator;
    fifo->alloc = alloc;
    fifo->dealloc = dealloc;

    fifo->head = 0;
    fifo->tail = 0;
    fifo->free = 0;

    return fifo;
}

void *mfifo_add( struct mfifo *fifo ) {

    const size_t n = sizeof(struct chunk) + fifo->chunk_size;

    struct chunk *oldtail = fifo->tail;

    // FIXME: get from pool first
    struct chunk *ch = fifo->alloc(fifo->allocator, n);

    fifo->tail = ch;
    ch->n = 0;
    ch->p = oldtail;

    if( oldtail ) {
        oldtail->n = ch;
    }

    if( !fifo->head ) {
        fifo->head = ch;
    }

    return ch->data;
}

void *mfifo_get( struct mfifo *fifo ) {

    if( !fifo->head ) {
        return 0;
    }

    struct chunk *h = fifo->head;

    fifo->head = fifo->head->n;

    if( fifo->head ) {
        fifo->head->p = 0;
    }

    // return item to pool or deallocate

    if( h ) {
        return h->data;
    }

    return 0;
}

void mfifo_iter_fwd( struct mfifo *fifo, void *cc, void (*fn)(void*,void*) ) {

    if( !fn ) {
        return;
    }

    struct chunk *it = fifo->head;
    for(; it; it = it->n ) {
        fn(cc, it->data);
    }
}

void mfifo_iter_back( struct mfifo *fifo, void *cc, void (*fn)(void*,void*) ) {

    if( !fn ) {
        return;
    }

    struct chunk *it = fifo->tail;
    for(; it; it = it->p ) {
        fn(cc, it->data);
    }
}


