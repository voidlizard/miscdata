#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include "slist.h"

/*#include <stdlib.h>*/
/*#include <string.h>*/
/*#include <unistd.h>*/

void init_char(void *cc, char *value) {
    char *c = (char*)cc;
    *value = *c;
}

int main(int _, char **__) {
    char pool[256 * slist_size(sizeof(char))];
    slist *free = slist_pool(pool, slist_size(sizeof(char)), sizeof(pool));
 
    printf("%08X\n", (unsigned long long)free);

    char c;
    slist *ls = slist_nil();
    for( c = 'A'; c <= 'Z'; c++) {
        ls = slist_cons(slist_alloc(&free, &c, init_char), ls);
    }

    slist *p = slist_nil();
    for(p = ls; p; p = p->next ) {
        printf("slist(%c)\n", *((char*)p->value));
    }

    printf("size of pool: %d %d\n", sizeof(pool), sizeof(pool)/slist_size(sizeof(char)));


    return 0; 
}

