#ifndef __MISCDATA_HASH_IP_H__
#define __MISCDATA_HASH_IP_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

struct sockaddr;

bool ip_addr_is_ip6(struct sockaddr *);
uint32_t ip_addr_hash(void *);
bool ip_addr_cmp(void *, void *);
void ip_addr_cpy(void *, void *);
void ip_addr_pretty(struct sockaddr *, char *, size_t);

#endif
