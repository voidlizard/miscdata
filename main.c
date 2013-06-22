#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct rtrie_ {
    struct rtrie_ *link;
    struct rtrie_ *sibling;
    char          *ka;
    char          *ke;
    char          *v;
} rtrie;


typedef void (*rtrie_cb)(void*, char *, char *, void*);

size_t rtrie_klen(char *ka, char *ke) {
    return (size_t)(ke - ka);
}

size_t rtrie_prefix_len(char *s, char *ka, char *ke) {
    size_t sl = strlen(s);
    char *se = s + sl;
    size_t pl = 0;
    for( ; *s && s < se && ka < ke && *s == *ka; s++, ka++ ) pl++;
    return pl;
}

bool rtrie_prefix_of(char *s, char *ka, char *ke, size_t *pl) {
    size_t l = rtrie_prefix_len(s, ka, ke);
    if( pl ) *pl = l;
    return  l > 0 && rtrie_klen(ka,ke) > strlen(s);
}

rtrie *rtrie_new(char *ka, char *ke, void *v) {
    rtrie *n = malloc(sizeof(rtrie));
    if( n ) {
        n->link = n->sibling = 0;
        n->ka = ka;
        n->ke = ke;
        n->v  = v;
    }
    return n;
}

rtrie *rtrie_assign(rtrie *t, char *ka, char *ke, void *v) {
    t->ka = ka;
    t->ke = ke;
    t->v  = v;
    return t;
}

bool rtrie_leaf(rtrie *t) {
    return !t->sibling && !t->link;
}

void rtrie_add(rtrie *t, char *sa, size_t len, void* v) {
    size_t pl = rtrie_prefix_len(sa,t->ka,t->ke);
    size_t kl = rtrie_klen(t->ka, t->ke);
    char  *se = sa + len;

    assert(t);

    if( rtrie_leaf(t) ) {
        rtrie_assign(t, sa, se, v);
        return;
    }

    if( !pl ) { // not a prefix
        // add at sibling
        if( !t->sibling ) {
            rtrie *n = rtrie_new(sa, se, v);
            t->sibling = n;
            return;
        }
        rtrie_add(t->sibling, sa, len, v);
        return;
    }

    if( pl == len && kl == len ) {
        t->v = v;
        return;
    }

    if( pl < kl ) { // new node
        rtrie *n = rtrie_new(t->ka + pl, t->ke, t->v);
        n->link = t->link;
        t->link = n;
        rtrie_assign(t, t->ka, t->ka - pl, 0);
    }
    
    rtrie_add(t, sa + pl, len - pl, v);
}

void rtrie_scan(rtrie *t, rtrie_cb fn, void *cc) {
}

int main(int _, char **__) {
    return 0;
}

