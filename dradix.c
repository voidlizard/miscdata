#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "dradix.h"

static char *const nullptr = "{nil}";
static void *const emptyptr = "{empty}";

#define safecall(v, f, ...)  ((f) ? (f(__VA_ARGS__)) : (v))
#define unit {} 

bool rtrie_emptyval(void *v) {
    return v == emptyptr;
}

static inline size_t rtrie_klen(char *ka, char *ke) {
    return (size_t)(ke - ka);
}

static size_t rtrie_prefix_len(char *s, char *ka, char *ke) {
    size_t sl = strlen(s);
    char *se = s + sl;
    size_t pl = 0;
    for( ; s < se && ka < ke && *s == *ka; s++, ka++ ) pl++;
    return pl < sl ? pl : sl;
}

static rtrie *rtrie_new(char *ka, char *ke, void *v) {
    rtrie *n = malloc(sizeof(rtrie));
    if( n ) {
        n->link = n->sibling = 0;
        n->ka = ka;
        n->ke = ke;
        n->v  = v;
    }
    return n;
}

rtrie *rtrie_nil() {
    return rtrie_new(nullptr,nullptr,emptyptr);
}

static rtrie *rtrie_assign(rtrie *t, char *ka, char *ke, void *v) {
    t->ka = ka;
    t->ke = ke;
    t->v  = v;
    return t;
}

bool rtrie_null(rtrie *t) {
    return t->ka == nullptr && !t->sibling && !t->link;
}

bool rtrie_empty(rtrie *t) {
    return t->v == emptyptr;
}

void rtrie_add(rtrie *t, char *sa, size_t len, void* v) {

    assert(t);

    size_t pl = rtrie_prefix_len(sa,t->ka,t->ke);
    size_t kl = rtrie_klen(t->ka, t->ke);
    char  *se = sa + len;

    if( ! *sa ) return; // prune empty strings

    if( rtrie_null(t) ) {
        rtrie_assign(t, sa, se, v);
        return;
    }

    if( !pl ) { // not a prefix
        // add at sibling
        if( !t->sibling ) {
            rtrie *n = rtrie_new(sa, se, v);
            n->sibling = t->sibling;
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
        rtrie_assign(t, t->ka, t->ka + pl, emptyptr);
        // prune empty strings
        if( ! *(sa + pl)  ) {
            t->v = v;
            return;
        }
    }

    if( !t->link ) t->link = rtrie_nil();

    rtrie_add(t->link, sa+pl, len-pl, v);
}

void rtrie_bfs(rtrie *t, void *cc, rtrie_cb cb) {
    if( !t ) return;
    safecall(unit, cb, cc, t->ka, t->ke, t->v);
    rtrie_bfs(t->sibling, cc, cb);
    rtrie_bfs(t->link, cc, cb);
}

void rtrie_dfs(rtrie *t, void *cc, rtrie_cb cb) {
    if( !t ) return;
    safecall(unit, cb, cc, t->ka, t->ke, t->v);
    rtrie_dfs(t->link, cc, cb);
    rtrie_dfs(t->sibling, cc, cb);
}

bool rtrie_lookup(rtrie *t, char *key, size_t len, rtrie **l, void* cc, rtrie_cb cb) {
    char *s  = (char*)key;

    if( !t ) {
        return false;
    }

    size_t pl = rtrie_prefix_len(s,t->ka,t->ke);

    if( !pl ) {
        return rtrie_lookup(t->sibling, key, len, l, cc, cb);
    } 

    // partial match

    size_t kl = rtrie_klen(t->ka,t->ke);

    if( pl == kl && !rtrie_empty(t) ) {
        *l = t;
        safecall(unit, cb, cc, t->ka, t->ke, t->v);
    }

    // full match
    if( len == kl && pl == kl && !rtrie_empty(t) ) {
        return true;
    }
    
    return rtrie_lookup(t->link, s + pl, len - pl, l, cc, cb);
}

// TODO: delete node
// TODO: iterate with prefix
// TODO: iterate with prefix

char* rtrie_tocstring(char *buf, size_t len, char *sa, char *se) {
    char *p = buf;
    size_t l = 0;
    for(; sa < se && l < len; sa++, p++) *p = *sa;
    *p = 0;
    return buf;
}

