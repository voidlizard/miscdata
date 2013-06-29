#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "dradix.h"

////// TESTS

void dump_node(void *cc, char *sa, char *se, void *v);

bool test_1() {
    rtrie *t = rtrie_nil();
 
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

    rtrie_free(t,0,0);

    return false;
}

bool test_2() {
    rtrie *t = rtrie_nil();
    
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

    rtrie_free(t,0,0);

    return false;
}


bool test_3() {
    rtrie *t = rtrie_nil();
    
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

    rtrie_free(t,0,0);

    return false;
}


bool test_4() {
    rtrie *t = rtrie_nil();
    
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

    rtrie_free(t,0,0);

    return false;
}

bool test_5() {
    rtrie *t = rtrie_nil();
    
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

    rtrie_free(t,0,0);

    return false;
}

bool test_6() {
    rtrie *t = rtrie_nil();
    
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

    rtrie_free(t,0,0);

    return false;
}


bool test_7() {
    rtrie *t = rtrie_nil();
    
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
            rtrie_tocstring(k, sizeof(k), n->ka, n->ke);
            v = n->v ? *(int*)n->v : -1;
        }
        printf("FOUND %s: %s (%s,%d) \n", (r?"TRUE":"FALSE"), buf[i].k, k, v);
    }

    rtrie_free(t,0,0);

    return false;
}

bool test_8() {
    rtrie *t = rtrie_nil();
    
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
            rtrie_tocstring(k, sizeof(k), n->ka, n->ke);
            v = n->v ? *(int*)n->v : -1;
        }
        printf("FOUND %s: %s (%s,%d) #%ul \n", (r?"TRUE":"FALSE"), buf[i].k, k, v, n);
    }

    rtrie_free(t,0,0);

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
    printf("test9_scan: (%s,)\n", cc->s); //rtrie_tocstring(tmp,255,sa,se));
}

void test10_cb(void *cc_, char *sa, char *se, void *v_) {
    int v = v_ ? *(int*)v_ : -1;
    printf("found partial match : %d\n", v);
}

bool test_10() {
    rtrie *t = rtrie_nil();
    
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
       , {  ".fsf.gro" }
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
            rtrie_tocstring(k, sizeof(buf), n->ka, n->ke);
            v = n->v ? *(int*)n->v : -1;
        }
        printf("FOUND %s: %s (%s,%d) #%ul \n", (r?"TRUE":"FALSE"), qq[i].q, k, v, n);
    }

    rtrie_free(t,0,0);

    return false;
}


bool test_11() {
    rtrie *t = rtrie_nil();
    
    struct kv { char k[64]; int v; } buf[] = {
         {  ".moc.elgoog",   1 }
       , {  ".ur.xednay",    2 }
       , {  ".",             3 }
    };

    int i = 0;
    for(i = 0; i < sizeof(buf)/sizeof(buf[0]); i++ ) {
            rtrie_add(t, buf[i].k, strlen(buf[i].k), &buf[i].v);
    }

    struct q { char q[64]; } qq[] = {
         {  ".fsf.gro" }
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
            rtrie_tocstring(k, sizeof(buf), n->ka, n->ke);
            v = n->v ? *(int*)n->v : -1;
        }
        printf("FOUND %s: %s (%s,%d) #%ul \n", (r?"TRUE":"FALSE"), qq[i].q, k, v, n);
    }

    rtrie_free(t,0,0);

    return false;
}



void test_12_cb(void *cc, rtrie *t) {
    printf("node#%ul\n", t);
}

bool test_12() {
    rtrie *t = rtrie_nil();

    char *s[] = { "ABAK"
                , "ZHABA"
                , "ABAKAN"
                , "ABA"
                , "XPEH"
                , "XPEHOBYХА"
                };
 
    rtrie_add(t, s[0], strlen(s[0]), 0);
    rtrie_add(t, s[1], strlen(s[1]), 0);
    rtrie_add(t, s[2], strlen(s[2]), 0);
    rtrie_add(t, s[3], strlen(s[3]), 0);
    rtrie_add(t, s[4], strlen(s[4]), 0);
    rtrie_add(t, s[5], strlen(s[5]), 0);

    rtrie_bfs_with_node(t,0,test_12_cb);

    rtrie_free(t,0,0);

    return false;
}



int main(int _, char **__) {
    test_1();
    test_2();
    test_3();
    test_4();
    test_5();
    test_6();
    test_7();
    test_8();
    test_10();
    test_11();
    test_12();
    return 0;
}



void dump_node(void *cc, char *sa, char *se, void *v) {
    char *buf = (char*)cc;
    int vv = v && !rtrie_emptyval(v) ? *(int*)v : -1;
    fprintf(stdout, "%s#%d\n", rtrie_tocstring(buf,128,sa,se), vv);
}

