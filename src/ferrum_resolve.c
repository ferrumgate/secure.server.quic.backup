#include "ferrum_resolve.h"

int32_t ferrum_resolve_sync(const char *domain, ferrum_resolve_type_t type,
                            ferrum_sockaddr_t **addr, size_t *len) {

  *len = 0;
  *addr = NULL;
  ferrum_log_info("resolving %s\n", domain);
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = type == A ? AF_INET : AF_INET6;
  hints.ai_flags |= AI_CANONNAME;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;
  struct addrinfo *result, *tmp;
  int res = getaddrinfo(domain, NULL, &hints, &result);
  if (res) {
    // ferrum_log_error("%s resolve failed:%s for type:%s\n", type == A ? "A" : "AAAA", domain, gai_strerror(res));
    return FERRUM_ERR_RESOLV;
  }
  size_t tcounter = 0;
  for (tmp = result; tmp != NULL; tmp = tmp->ai_next)
    tcounter++;
  ferrum_sockaddr_t *ptraddr = ferrum_new_array(ferrum_sockaddr_t, tcounter);
  size_t counter = 0;
  for (tmp = result; tmp != NULL; tmp = tmp->ai_next) {
    ferrum_util_addr_to_ferrum_addr(tmp->ai_addr, ptraddr + (counter++));
  }
  freeaddrinfo(result);
  *addr = ptraddr;
  *len = tcounter;
  return FERRUM_SUCCESS;
}