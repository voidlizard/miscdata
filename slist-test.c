#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "slist.h"


void init_char(void *cc, char *value) {
    char *c = (char*)cc;
    *value = *c;
}

void test_slist_1(void) {

    char pool[256 * slist_size(sizeof(char))];
    slist *free = slist_pool(pool, slist_size(sizeof(char)), sizeof(pool));

    char c;
    slist *ls = slist_nil();
    for( c = 'A'; c <= 'Z'; c++) {
        ls = slist_cons(slist_alloc(&free, &c, init_char), ls);
    }

    slist *p = slist_nil();
    for(p = ls; p; p = p->next ) {
        fprintf(stdout, "slist(%c)\n", *((char*)p->value));
    }

    fprintf(stdout, "size of pool: %zu %zu\n", sizeof(pool), sizeof(pool)/slist_size(sizeof(char)));

}

static void print_item(void *cc, void *e) {
    int *sum = (int*)cc;
    (*sum)++;
    fprintf(stdout, "item(%c) sum: %d\n", *(char*)e, *sum);
}

void test_slist_2(void) {

    char pool[256 * slist_size(sizeof(char))];
    slist *free = slist_pool(pool, slist_size(sizeof(char)), sizeof(pool));

    char c;
    slist *ls = slist_nil();
    for( c = 'A'; c <= 'Z'; c++) {
        ls = slist_cons(slist_alloc(&free, &c, init_char), ls);
    }

    int sum = 0;
    slist_foreach(ls, &sum, print_item);

}

static bool filt_leq(void *cc, void *item) {
    return *(char*)item <= *(char*)cc;
}

static void destroy_none(void *c, slist *e) {
    fprintf(stdout, "destroying (%c)\n", *(char*)e->value);
}

static void destroy_free(void *c, slist *e) {
    fprintf(stdout, "freeing (%c)\n", *(char*)e->value);
    free(e);
}

void test_slist_filt_destructive_1(void) {

    char pool[256 * slist_size(sizeof(char))];
    slist *free = slist_pool(pool, slist_size(sizeof(char)), sizeof(pool));

    char c;
    slist *ls = slist_nil();
    for( c = 'A'; c <= 'Z'; c++) {
        ls = slist_cons(slist_alloc(&free, &c, init_char), ls);
    }

    int sum = 0;
    slist_foreach(ls, &sum, print_item);

    char v = 'F';
    slist_filt_destructive(&ls, &v, filt_leq, 0, destroy_none);

    fprintf(stdout, "filtered\n");

    slist_foreach(ls, &sum, print_item);

}

void test_slist_filt_destructive_2(void) {

    char c;
    slist *ls = slist_nil();
    for( c = 'A'; c <= 'Z'; c++) {
        slist *nl = malloc(slist_size(4));
        *(char*)nl->value = c;
        ls = slist_cons(nl, ls);
    }

    int sum = 0;
    slist_foreach(ls, &sum, print_item);

    char v = 'G';
    slist_filt_destructive(&ls, &v, filt_leq, 0, destroy_free);

    fprintf(stdout, "filtered\n");

    slist_foreach(ls, &sum, print_item);

    slist_filt_destructive(&ls, 0, 0, 0, destroy_free);
}

void test_slist_partition_destructive_1(void) {

    char c;
    slist *ls = slist_nil();
    for( c = 'A'; c <= 'Z'; c++) {
        slist *nl = malloc(slist_size(4));
        *(char*)nl->value = c;
        ls = slist_cons(nl, ls);
    }

    int sum = 0;
    slist_foreach(ls, &sum, print_item);

    char v = 'M';
    slist *rest = slist_nil();
    slist_partition_destructive( &ls
                               , &rest
                               , &v
                               , filt_leq);

    fprintf(stdout, "partitioned -- match\n");

    slist_foreach(ls, &sum, print_item);

    fprintf(stdout, "partitioned -- rest\n");

    slist_foreach(rest, &sum, print_item);

    slist_filt_destructive(&ls, 0, 0, 0, destroy_free);
    slist_filt_destructive(&rest, 0, 0, 0, destroy_free);
}

