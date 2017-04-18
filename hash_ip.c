#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

#include "hash_ip.h"
#include "hash_fletcher32.h"

bool ip_addr_is_ip6(struct sockaddr *sa) {
    return ( sa->sa_family == AF_INET6 );
}

uint32_t ip_addr_hash(void *cc) {
    struct sockaddr *sa = (struct sockaddr*)cc;
    uint32_t hash = 0;

    if( ip_addr_is_ip6(sa) ) {
        struct sockaddr_in6 *sin = (struct sockaddr_in6 *)sa;
        hash = hash_fletcher32( (uint16_t *)(sin->sin6_addr.s6_addr)
                              , sizeof(sin->sin6_addr.s6_addr) / sizeof(uint16_t) );
    } else {
        struct sockaddr_in *sin = (struct sockaddr_in*)sa;
        hash = (uint32_t)(sin->sin_addr.s_addr);
    }

    return hash;
}

bool ip_addr_cmp(void *a1, void *a2) {

    struct sockaddr *sa1 = (struct sockaddr *)a1;
    struct sockaddr *sa2 = (struct sockaddr *)a2;

    if( !ip_addr_is_ip6(sa1) && !ip_addr_is_ip6(sa2) ) {
        return ( (((struct sockaddr_in*)sa1)->sin_addr.s_addr) ==
                 (((struct sockaddr_in*)sa2)->sin_addr.s_addr) );
    }

    if( ip_addr_is_ip6(sa1) && ip_addr_is_ip6(sa2) ) {
        int r = memcmp( ((struct sockaddr_in6*)sa1)->sin6_addr.s6_addr
                      , ((struct sockaddr_in6*)sa2)->sin6_addr.s6_addr
                      , sizeof(((struct sockaddr_in6*)sa1)->sin6_addr.s6_addr) );
        return !r;
    }

    return false;
}

void ip_addr_cpy(void *dst, void *src) {
    size_t size;

    // TODO chk dst??
    if( ip_addr_is_ip6(src) ) {
        size = sizeof(struct sockaddr_in6);
    } else {
        size = sizeof(struct sockaddr_in);
    }

    memcpy(dst, src, size);
}

void ip_addr_pretty(struct sockaddr *sa, char *buf, size_t size) {

	if( ip_addr_is_ip6(sa) ) {
		inet_ntop(AF_INET6, &((struct sockaddr_in6 *)sa)->sin6_addr, buf, size);
	} else {
		inet_ntop(AF_INET, &((struct sockaddr_in *)sa)->sin_addr, buf, size);
	}
}
