#ifndef __FERRUM_RESOLVE_H__
#define __FERRUM_RESOLVE_H__

#include "ferrum_common.h"
#include "ferrum_log.h"

typedef enum ferrum_resolve_type {
  A,
  AAAA
} ferrum_resolve_type_t;

int32_t ferrum_resolve_sync(const char *domain, ferrum_resolve_type_t type,
                            ferrum_sockaddr_t **addr, size_t *len);

#endif