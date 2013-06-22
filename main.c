#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define safecall(v, f, ...)  ((f) ? (f(__VA_ARGS__)) : (v))
#define unit {} 


void dump_node(void *cc, char *sa, char *se, void *v);

static char *const leafptr = "{nil}";

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
    for( ; s <= se && ka <= ke && *s == *ka; s++, ka++ ) pl++;
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
    return rtrie_new(leafptr,leafptr,0);
}

rtrie *rtrie_assign(rtrie *t, char *ka, char *ke, void *v) {
    t->ka = ka;
    t->ke = ke;
    t->v  = v;
    return t;
}

bool rtrie_leaf(rtrie *t) {
    return t->ka == leafptr && !t->sibling && !t->link;
}

void rtrie_add(rtrie *t, char *sa, size_t len, void* v) {

    assert(t);

    size_t pl = rtrie_prefix_len(sa,t->ka,t->ke);
    size_t kl = rtrie_klen(t->ka, t->ke);
    char  *se = sa + len;

/*    if( ! *sa ) return; // prune empty strings*/

    printf("adding '%s'\n",sa);

    printf("0 prefix: %s %s  %d %d\n",sa, t->ka, pl, kl);

    if( rtrie_leaf(t) ) {
        printf("adding leaf %s %d\n",sa,len);
        rtrie_assign(t, sa, se, v);
        return;
    }

    if( !pl ) { // not a prefix
        // add at sibling
        printf("adding sibling %s\n",sa);
        if( !t->sibling ) {
            printf("new sibling %s\n",sa);
            rtrie *n = rtrie_new(sa, se, v);
            n->sibling = t->sibling;
            t->sibling = n;
            return;
        }
        rtrie_add(t->sibling, sa, len, v);
        return;
    }

    if( pl == len && kl == len ) {
        printf("just replace %s : %s with %s\n", sa, t->ka, sa);
        t->v = v;
        return;
    }

    if( pl < kl ) { // new node
        printf("split node (pl %d) (kl %d) %s ( %s )\n", pl, kl, t->ka, sa);
        rtrie *n = rtrie_new(t->ka + pl, t->ke, t->v);
        n->link = t->link;
        t->link = n;
        // TODO: WTF?
/*        rtrie_assign(t, t->ka, t->ka + pl - 1, 0);*/
        rtrie_assign(t, t->ka, t->ka + pl - 1, 0);
        // prune empty strings
/*        if( ! *(sa + pl)  ) {*/
/*            t->v = v;*/
/*            return;*/
/*        }*/
    }

    if( !t->link ) t->link = rtrie_nil();
  
    printf("wtf: %s %s %d\n",t->ka, sa, pl);
    printf("adding remain: %d %d '%s' to %s\n", len, pl, sa + pl, t->ka);
    rtrie_add(t->link, sa+pl, len-pl, v);
}

void rtrie_bfs(rtrie *t, void *cc, rtrie_cb cb) {
    if( !t ) return;
    safecall(unit, cb, cc, t->ka, t->ke, t->v);
    rtrie_bfs(t->sibling, cc, cb);
    rtrie_bfs(t->link, cc, cb);
}

void rtrie_lookup_go(rtrie *t) {
}

void rtrie_lookup(rtrie *t, char const* key) {
}

bool test_1(rtrie *t) {
    (void)t;
    
    struct kv { char k[32]; int v; } buf[] = {
          {  "A",  31 }
        , {  "AB", 32 }
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
          {  "ABCD",  31 }
        , {  "QQQ",   32 }
        , {  "ABK",   33 }
        , {  "QQQE",  34 }
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
/*       , {  "CCD",  3 }*/
/*       , {  "AABC", 4 }*/
/*       , {  "",     5 }*/
/*       , {  "",     7 }*/
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
    };

    int i = 0;
    for(i = 0; i < sizeof(buf)/sizeof(buf[0]); i++ ) {
        rtrie_add(t, buf[i].k, strlen(buf[i].k), &buf[i].v);
    }

    char tmp[256];
    rtrie_bfs(t, tmp, dump_node);

    return false;
}



int main(int _, char **__) {
/*    test_1(rtrie_nil());*/
/*    test_2(rtrie_nil());*/
/*    test_3(rtrie_nil());*/
/*    test_4(rtrie_nil());*/
    test_5(rtrie_nil());
/*    test_6(rtrie_nil());*/
    return 0;
}


void dump_node(void *cc, char *sa, char *se, void *v) {
    char *buf = (char*)cc;
    char *p = buf;
    size_t l = 0;
    for(; sa <= se && l < 127; p++, sa++ ) *p = *sa;
     *p++ = 0;
    int vv = v ? *(int*)v : -1;
    fprintf(stdout, "%s#%d\n", buf, vv);
}

