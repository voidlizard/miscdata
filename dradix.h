#ifndef __dradix_h
#define __dradix_h

#include <stddef.h>

typedef struct rtrie_ {
    struct rtrie_ *link;
    struct rtrie_ *sibling;
    char          *ka;
    char          *ke;
    void          *v;
} rtrie;

typedef void (*rtrie_cb)(void*, char *, char *, void*);

char* rtrie_tocstring(char *buf, size_t len, char *sa, char *se);
bool rtrie_emptyval(void*);
rtrie *rtrie_nil(); 
bool rtrie_null(rtrie*); 
void rtrie_add(rtrie*, char *, size_t, void*);
void rtrie_bfs(rtrie*, void *, rtrie_cb); 
void rtrie_dfs(rtrie*, void *, rtrie_cb);
bool rtrie_lookup(rtrie*, char *, size_t, rtrie**, void*, rtrie_cb);

#endif

