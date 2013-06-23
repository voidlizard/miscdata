#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define safecall(v, f, ...)  ((f) ? (f(__VA_ARGS__)) : (v))
#define unit {} 

void dump_node(void *cc, char *sa, char *se, void *v);
char* snode(char *buf, size_t len, char *sa, char *se); 


static char *const nullptr = "{nil}";
static void *const emptyptr = "{empty}";

typedef struct rtrie_ {
    struct rtrie_ *link;
    struct rtrie_ *sibling;
    char          *ka;
    char          *ke;
    void          *v;
} rtrie;

typedef void (*rtrie_cb)(void*, char *, char *, void*);

size_t rtrie_klen(char *ka, char *ke) {
    return (size_t)(ke - ka);
}

size_t rtrie_prefix_len(char *s, char *ka, char *ke) {
    size_t sl = strlen(s);
    char *se = s + sl;
    size_t pl = 0;
    for( ; s < se && ka < ke && *s == *ka; s++, ka++ ) pl++;
    return pl < sl ? pl : sl;
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

rtrie *rtrie_nil() {
    return rtrie_new(nullptr,nullptr,emptyptr);
}

rtrie *rtrie_assign(rtrie *t, char *ka, char *ke, void *v) {
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
    char *se = s + len;

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
    if( len == kl && pl == kl && t->v != emptyptr ) {
        return true;
    }
    
    return rtrie_lookup(t->link, s + pl, len - pl, l, cc, cb);
}

bool test_1(rtrie *t) {
    (void)t;
    
    struct kv { char k[32]; int v; } buf[] = {
          {  "A",   31 }
        , {  "AB",  32 }
        , {  "AAC", 33 }
    };

    int i = 0;
    for(i = 0; i < sizeof(buf)/sizeof(buf[0]); i++ ) {
        rtrie_add(t, buf[i].k, strlen(buf[i].k), &buf[i].v);
    }

    char tmp[256];
    rtrie_bfs(t, tmp, dump_node);

    return false;
}

bool test_2(rtrie *t) {
    (void)t;
    
    struct kv { char k[32]; int v; } buf[] = {
          {  "AB",     1 }
        , {  "A",      2 }
        , {  "AB",     3 }
    };

    int i = 0;
    for(i = 0; i < sizeof(buf)/sizeof(buf[0]); i++ ) {
        rtrie_add(t, buf[i].k, strlen(buf[i].k), &buf[i].v);
    }

    char tmp[256];
    rtrie_bfs(t, tmp, dump_node);

    return false;
}


bool test_3(rtrie *t) {
    (void)t;
    
    struct kv { char k[32]; int v; } buf[] = {
          {  "AAAAK",  1 }
        , {  "AAAAKZ", 2 }
        , {  "AAAAKM", 3 }
    };

    int i = 0;
    for(i = 0; i < sizeof(buf)/sizeof(buf[0]); i++ ) {
        rtrie_add(t, buf[i].k, strlen(buf[i].k), &buf[i].v);
    }

    char tmp[256];
    rtrie_bfs(t, tmp, dump_node);

    return false;
}


bool test_4(rtrie *t) {
    (void)t;
    
    struct kv { char k[32]; int v; } buf[] = {
           {  "AAAAB",  1 }
         , {  "JOPA",   2 }
         , {  "KITA",   3 }
         , {  "AK47",   4 }
         , {  "JORA",   5 }
    };

    int i = 0;
    for(i = 0; i < sizeof(buf)/sizeof(buf[0]); i++ ) {
        rtrie_add(t, buf[i].k, strlen(buf[i].k), &buf[i].v);
    }

    char tmp[256];
    rtrie_bfs(t, tmp, dump_node);

    return false;
}

bool test_5(rtrie *t) {
    (void)t;
    
    struct kv { char k[32]; int v; } buf[] = {
         {  "AABA", 1 }
       , {  "AAB",  2 }
       , {  "CCD",  3 }
       , {  "AABC", 4 }
       , {  "",     5 }
       , {  "",     7 }
    };

    int i = 0;
    for(i = 0; i < sizeof(buf)/sizeof(buf[0]); i++ ) {
        rtrie_add(t, buf[i].k, strlen(buf[i].k), &buf[i].v);
    }

    char tmp[256];
    rtrie_bfs(t, tmp, dump_node);

    return false;
}

bool test_6(rtrie *t) {
    (void)t;
    
    struct kv { char k[32]; int v; } buf[] = {
         {  "AC",    1 }
       , {  "AABA",  2 }
       , {  "AC",    3 }
       , {  "AC",    4 }
       , {  "AC",    5 }
       , {  "A",     6 }
    };

    int i = 0;
    for(i = 0; i < sizeof(buf)/sizeof(buf[0]); i++ ) {
        rtrie_add(t, buf[i].k, strlen(buf[i].k), &buf[i].v);
    }

    char tmp[256];
    rtrie_bfs(t, tmp, dump_node);

    return false;
}


bool test_7(rtrie *t) {
    (void)t;
    
    struct kv { char k[32]; int v; } buf[] = {
         {  "JOPA",    1 }
       , {  "KITA",    2 }
       , {  "PECHEN",  3 }
       , {  "TRESKI",  4 }
    };

    int i = 0;
    for(i = 0; i < sizeof(buf)/sizeof(buf[0]); i++ ) {
        rtrie_add(t, buf[i].k, strlen(buf[i].k), &buf[i].v);
    }

    for(i = 0; i < sizeof(buf)/sizeof(buf[0]); i++ ) {
        char k[32];
        rtrie *n = 0;
        bool r = rtrie_lookup(t, buf[i].k, strlen(buf[i].k), &n, 0, 0);
        int  v = -1;
        if( n ) {
            snode(k, sizeof(k), n->ka, n->ke);
            v = n->v ? *(int*)n->v : -1;
        }
        printf("FOUND %s: %s (%s,%d) \n", (r?"TRUE":"FALSE"), buf[i].k, k, v);
    }

    return false;
}

bool test_8(rtrie *t) {
    (void)t;
    
    struct kv { char k[32]; int v; } buf[] = {
         {  "A",       1 }
       , {  "ABAK",    2 }
       , {  "BOO",    10 }
       , {  "ABAKAN",  3 }
       , {  "BABA",    4 }
       , {  "AB",      0 }
       , {  "A",       0 }
       , {  "B",      -2 }
       , {  "AR",     -1 }
       , {  "ARBAN",  -2 }
       , {  "MOMOMO", -3 }
       , {  "ZBABA",  -4 }
       , {  "ABAKANBABAI", -5 }
    };

    int i = 0;
    for(i = 0; i < sizeof(buf)/sizeof(buf[0]); i++ ) {
        if( buf[i].v > 0 ) {
            rtrie_add(t, buf[i].k, strlen(buf[i].k), &buf[i].v);
        }
    }

    char tmp[256];
    rtrie_bfs(t, tmp, dump_node);

    for(i = 0; i < sizeof(buf)/sizeof(buf[0]); i++ ) {
        char k[32];
        rtrie *n = 0;
        bool r = rtrie_lookup(t, buf[i].k, strlen(buf[i].k), &n, 0, 0);
        int  v = -1;

        if( n ) {
            snode(k, sizeof(k), n->ka, n->ke);
            v = n->v ? *(int*)n->v : -1;
        }
        printf("FOUND %s: %s (%s,%d) #%ul \n", (r?"TRUE":"FALSE"), buf[i].k, k, v, n);
    }

    return false;
}


struct test9_scan_cc {
    char *s;
    char *p;
    char *pe;
};

void test9_scan(void *cc_, char *sa, char *se, void *v) {
    struct test9_scan_cc *cc = cc_;
    assert(cc);
    char *p = cc->p;
    char *pe = cc->pe;
    for(; p < pe && sa < se; p++, sa++ ) *p = *sa;
    *p = 0;
    printf("test9_scan: (%s,)\n", cc->s); //snode(tmp,255,sa,se));
}

void test10_cb(void *cc_, char *sa, char *se, void *v_) {
    int v = v_ ? *(int*)v_ : -1;
    printf("found partial match : %d\n", v);
}

bool test_10(rtrie *t) {
    (void)t;
    
    struct kv { char k[64]; int v; } buf[] = {
         {  ".",             1 }
       , {  ".moc",          2 }
       , {  ".moc.elgoog",   3 }
       , {  ".moc.elgoog.a", 4 }
       , {  ".ur.xednay",    6 }
       , {  ".ur",           5 }
    };

    int i = 0;
    for(i = 0; i < sizeof(buf)/sizeof(buf[0]); i++ ) {
            rtrie_add(t, buf[i].k, strlen(buf[i].k), &buf[i].v);
    }

    struct q { char q[64]; } qq[] = {
         {  ".ur.ay" }
       , {  ".moc.oob" }
       , {  ".gro.fbo" }
       , {  ".moc.abcd" }
       , {  ".moc.elgoog.c" }
       , {  ".moc.elgoog.a.d" }
       , {  ".oof" }
    };

    char tmp[256];
    rtrie_bfs(t, tmp, dump_node);
    printf("\n\n");

    for(i = 0; i < sizeof(qq)/sizeof(qq[0]); i++ ) {
        char k[64];
        rtrie *n = 0;
        printf("LOOKUP %s\n", qq[i].q);
        bool r = rtrie_lookup(t, qq[i].q, strlen(qq[i].q), &n, 0, test10_cb);
        int  v = -1;
        if( n ) {
            snode(k, sizeof(buf), n->ka, n->ke);
            v = n->v ? *(int*)n->v : -1;
        }
        printf("FOUND %s: %s (%s,%d) #%ul \n", (r?"TRUE":"FALSE"), qq[i].q, k, v, n);
    }

    return false;
}


int main(int _, char **__) {
/*    test_1(rtrie_nil());*/
/*    test_2(rtrie_nil());*/
/*    test_3(rtrie_nil());*/
/*    test_4(rtrie_nil());*/
/*    test_5(rtrie_nil());*/
/*    test_6(rtrie_nil());*/
/*    test_7(rtrie_nil());*/
/*    test_8(rtrie_nil());*/
/*    test_9(rtrie_nil());*/
    test_10(rtrie_nil());
    return 0;
}


char* snode(char *buf, size_t len, char *sa, char *se) {
    char *p = buf;
    size_t l = 0;
    for(; sa < se && l < len; sa++, p++) *p = *sa;
    *p = 0;
    return buf;
}

void dump_node(void *cc, char *sa, char *se, void *v) {
    char *buf = (char*)cc;
    int vv = v && v != emptyptr ? *(int*)v : -1;
    fprintf(stdout, "%s#%d\n", snode(buf,128,sa,se), vv);
}

