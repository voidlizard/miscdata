#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "t/test-suite.h"

static struct test_ {
    void (*test_fun)(void);
    const char *name;
} tests[] = {
     {  test_dradix_1,   "test_dradix_1"   }
    ,{  test_dradix_1_1, "test_dradix_1_1" }
    ,{  test_dradix_2,   "test_dradix_2"   }
    ,{  test_dradix_3,   "test_dradix_3"   }
    ,{  test_dradix_4,   "test_dradix_4"   }
    ,{  test_dradix_5,   "test_dradix_5"   }
    ,{  test_dradix_6,   "test_dradix_6"   }
    ,{  test_dradix_7,   "test_dradix_7"   }
    ,{  test_dradix_8,   "test_dradix_8"   }
    ,{  test_dradix_10,  "test_dradix_10"  }
    ,{  test_dradix_11,  "test_dradix_11"  }
    ,{  test_dradix_12,  "test_dradix_12"  }
    ,{  test_dradix_13,  "test_dradix_13"  }
    ,{  test_dradix_14,  "test_dradix_14"  }
    ,{  test_dradix_15,  "test_dradix_15"  }
    ,{  test_dradix_16,  "test_dradix_16"  }
    ,{  test_dradix_17,  "test_dradix_17"  }
    ,{  test_hash_create_1, "test_hash_create_1"  }
    ,{  test_hash_create_2, "test_hash_create_2 (str key)" }
    ,{  test_hash_alter_1, "test_hash_alter_1" }
    ,{  0,        ""         }
};

void tests_run(int n) {
    int k = sizeof(tests)/sizeof(tests[0]);
    int i = n >= 0 ? n % k : 0;
    int l = n >= 0 ? i + 1 : k;
    for(; i < l && tests[i].test_fun; i++) {
        fprintf(stderr, "\n=== TEST STARTED  (%s)\n", tests[i].name );
        tests[i].test_fun();
        fprintf(stderr, "\n=== TEST FINISHED (%s)\n", tests[i].name );
    }
}

int main(int argc, char **argv) {

    if( argc < 2 ) {
        tests_run(-1);
        return 0;
    }

    if( !strncmp("list", argv[1], strlen("list")) ) {
        int i = 0;
        int k = sizeof(tests)/sizeof(tests[0]);
        for(i = 0; i < k && tests[i].test_fun; i++) {
            fprintf(stderr, "%3d %s\n", i, tests[i].name );
        }
        return 0;
    }

    char *e = argv[1];
    long n = strtol(argv[1], &e, 10);
    if( e > argv[1] ) {
        tests_run(n);
        return 0;
    }

    return -1;
}

