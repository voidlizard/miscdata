#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "mfifo.h"


static void *__alloc(void *cc, size_t n) {
    return malloc(n);
}

static void __dealloc(void *cc, void *mem) {
    free(mem);
}

static void __print_int(void *to, void *v) {
    fprintf(to, "%02x ", *(int*)v );
}

void test_mfifo_create_1(void) {
    char mem[mfifo_size()];

    struct mfifo *fifo = mfifo_create( mem
                                     , sizeof(mem)
                                     , sizeof(int)
                                     , 10
                                     , 0
                                     , __alloc
                                     , __dealloc
                                     );


    size_t i = 0;
    for(; i < 10; i++ ) {

        int *v = mfifo_add(fifo);

        if( v ) {
            *v = (int)i;
        }
    }

    fprintf(stdout, "fifo: %s\n", fifo ? "exists" : "not exists");

    mfifo_iter_fwd(fifo, stdout, __print_int);
    fprintf(stdout, "\n");
    mfifo_iter_back(fifo, stdout, __print_int);
    fprintf(stdout, "\n");
}



void test_mfifo_create_2(void) {
    char mem[mfifo_size()];

    struct mfifo *fifo = mfifo_create( mem
                                     , sizeof(mem)
                                     , sizeof(int)
                                     , 16
                                     , 0
                                     , __alloc
                                     , __dealloc
                                     );

    size_t q = 0;

    for(; q < 16; q++ ) {

        size_t i = 0;
        for(; i < q; i++ ) {

            int *v = mfifo_add(fifo);

            if( v ) {
                *v = (int)i;
            }
        }

        int *z = 0;

        while( (z = mfifo_get(fifo)) )  {
            fprintf(stdout, "%02x ", *z);
        }
        fprintf(stdout, "\n");
    }

}


