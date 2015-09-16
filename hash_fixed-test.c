#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

#include "hash_fixed.h"
#include "hash_uint32.h"

#include "hash_test_common.h"

#define words(x) ((x)/sizeof(void*))

void test_hash_fixed_1(void) {

    const size_t bkt = 32;

    char mem[hash_mem_size_fixed(bkt, 100, sizeof(uint32_t), sizeof(uint32_t))];

    fprintf(stderr, "mem size: %zu\n\n", sizeof(mem));

    struct hash *h = hash_create_fixed( sizeof(mem)
                                      , mem
                                      , sizeof(uint32_t)
                                      , sizeof(uint32_t)
                                      , bkt
                                      , uint32_hash
                                      , uint32_eq
                                      , uint32_cpy
                                      , uint32_cpy );

    fprintf(stdout, "hash allocated? %s\n\n", h ? "yes" : "no");

    size_t i = 0;
    for(;;i++) {
        uint32_t k = i;
        uint32_t v = i;
        if( !hash_add(h, &k, &v) ) {
            break;
        }
    }

    fprintf(stdout, "added something? %s\n\n", i > 0 ? "yes" : "no");

    // platform-dependend
    fprintf(stderr, "added %zu\n\n", i);

    size_t j = 0;
    for(; j < i; j++ ) {
        uint32_t k = j;
        if( !hash_get(h, &k) ) {
            break;
        }
    }

    fprintf(stdout, "found all ? %s\n\n", i == j ? "yes" : "no");

    print_hash_stat(h);

    hash_filter(h, 0, 0);

    fprintf(stdout, "\nfiltered off\n\n");

    print_hash_stat(h);

    for(i=0;;i++) {
        uint32_t k = i;
        uint32_t v = i;
        if( !hash_add(h, &k, &v) ) {
            break;
        }
    }

    // platform-dependend
    fprintf(stdout, "\nadded: %zu\n\n", i);

    print_hash_stat(h);

    hash_destroy(h);
/*    const_mem_pool_destroy(pool);*/
}

