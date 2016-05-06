#ifndef __MFIFO_H__
#define __MFIFO_H__

#include <stddef.h>

struct mfifo;

const size_t mfifo_size(void);

struct mfifo *mfifo_create( void *mem
                          , size_t memsize
                          , size_t chunk_size
                          , size_t  free_chunks_max
                          , void *allocator
                          , void *(*alloc)(void *, size_t)
                          , void (*dealloc)(void*,void*)
                          );

void *mfifo_add( struct mfifo *fifo );
void *mfifo_get( struct mfifo *fifo );

void mfifo_iter_fwd( struct mfifo *fifo, void *cc, void (*fn)(void*,void*) );
void mfifo_iter_back( struct mfifo *fifo, void *cc, void (*fn)(void*,void*) );

#endif
