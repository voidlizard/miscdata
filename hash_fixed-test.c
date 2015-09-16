#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

#include "hash_fixed.h"
#include "hash_uint32.h"

#include "hash_test_common.h"

#define words(x) ((x)/sizeof(void*))


void test_hash_fixed_1(void) {
    char mem[2048];

    struct hash *h = hash_create_fixed( sizeof(mem)
                                      , mem
                                      , sizeof(uint32_t)
                                      , sizeof(uint32_t)
                                      , 32
                                      , uint32_hash
                                      , uint32_eq
                                      , uint32_cpy
                                      , uint32_cpy );

    fprintf(stdout, "hash allocated? %s\n", h ? "yes" : "no");

    size_t i = 0;
    for(;;i++) {
        uint32_t k = i;
        uint32_t v = i;
        if( !hash_add(h, &k, &v) ) {
            break;
        }
    }

    fprintf(stdout, "added something? %s\n", i > 0 ? "yes" : "no");

    // platform-dependend
    fprintf(stderr, "added %zu\n", i);

    size_t j = 0;
    for(; j < i; j++ ) {
        uint32_t k = j;
        if( !hash_get(h, &k) ) {
            break;
        }
    }

    fprintf(stdout, "found all ? %s\n", i == j ? "yes" : "no");

    print_hash_stat(h);

    hash_filter(h, 0, 0);

    fprintf(stdout, "\nfiltered\n");

    print_hash_stat(h);

    for(i=0;;i++) {
        uint32_t k = i;
        uint32_t v = i;
        if( !hash_add(h, &k, &v) ) {
            break;
        }
    }

    // platform-dependend
    fprintf(stdout, "\nadded: %zu\n", i);

    hash_destroy(h);
/*    const_mem_pool_destroy(pool);*/
}

