#ifndef __FERRUM_COMMON_H__
#define __FERRUM_COMMON_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdarg.h>
#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "uv.h"
#include "openssl/err.h"
#include "openssl/ssl.h"
#include "openssl/conf.h"
#include "openssl/engine.h"
#include "lsquic/lsquic.h"

#define TRUE 1
#define FALSE 0
/**
 * @brief errors and success, errors < 0
 */
#define FERRUM_SUCCESS 0

/*
 * @brief ip max string len
 */
#define FERRUM_IP_STR_LEN 64
#define FERRUM_PASS_STR_LEN 128

#define FERRUM_DOMAIN_LEN 2048
#define FERRUM_HOSTNAME_LEN 64
#define FERRUM_MAX_ENV_LEN 64
#define FERRUM_PORT_STR_LEN 8
#define FERRUM_HOST_STR_LEN 128
#define FERRUM_NAME_STR_LEN 128
#define FERRUM_IP_PORT_STR_LEN 96

#define FERRUM_TLS_KEY_LEN 128
#define FERRUM_CA_VERIFY_PATH_MAX_LEN 1024
#define FERRUM_TLS_ALPN_MAX_LEN 128

#define FERRUM_ERR_UV -10000
#define FERRUM_ERR -1000
#define FERRUM_ERR_BAD_ARGUMENT -1001
#define FERRUM_ERR_RESOLV -1002

#define ferrum_kill_current_process(n) exit(n)
/* @brief allocation methods */
#define ferrum_malloc(x) malloc(x)
#define ferrum_free(x) free(x)
#define ferrum_realloc(x, y) realloc(x, y)
#define ferrum_calloc(x, y) calloc(x, y)
#define ferrum_free_if_not_null_and_set_null(x) \
  if (x)                                        \
    free(x);                                    \
  x = NULL

#define ferrum_new1(x) ferrum_malloc(sizeof(x))

#define ferrum_constructor(x, y)          \
  if (!x) {                               \
    ferrum_log_fatal("malloc problem\n"); \
    exit(1);                              \
  }                                       \
  ferrum_fill_zero(x, sizeof(y));

#define ferrum_new2(y, x) \
  y x;                    \
  ferrum_fill_zero(&x, sizeof(y));

#define ferrum_new3(y, x) \
  y x;                    \
  ferrum_fill_zero(&x, sizeof(y));

#define ferrum_new4(y, x)                           \
  y *x = ferrum_malloc(sizeof(y));                  \
  ferrum_if_is_null_then_die(x, "malloc problem\n") \
      ferrum_fill_zero(x, sizeof(y));

#define ferrum_malloc2(x, y)                        \
  x = ferrum_malloc(y);                             \
  ferrum_if_is_null_then_die(x, "malloc problem\n") \
      ferrum_fill_zero(x, y);

#define ferrum_new_array(x, len) malloc(sizeof(x) * (len))
#define ferrum_fill_zero(x, size) memset((x), 0, (size))
#define ferrum_cast(x, y) ((y)(x))
#define ferrum_const_cast(x, y) ((y)(x))

#define ferrum_unused(x) (void)(x)
#define ferrum_if_is_null_then_die(x, y) \
  if (!x) {                              \
    ferrum_log_fatal(y);                 \
    exit(1);                             \
  }

#define ssizeof(x) ferrum_cast(sizeof(x), int32_t)

#define ferrum_cast_to_uint8ptr(x) ferrum_cast(x, uint8_t *)
#define ferrum_cast_to_const_uint8ptr(x) ferrum_cast(x, const uint8_t *)
#define ferrum_cast_to_charptr(x) ferrum_cast(x, char *)
#define ferrum_cast_to_const_charptr(x) ferrum_cast(x, const char *)
#define ferrum_cast_to_sockaddr(x) ferrum_cast(x, struct sockaddr *)

#define public_
#define private_
#define readonly_
#define protected_
#define internal_

/**
 * @brief socket address union
 *
 */
typedef union ferrum_sockaddr {
  struct sockaddr base;
  struct sockaddr_in v4;
  struct sockaddr_in6 v6;
} ferrum_sockaddr_t;

//////////////// rebrick clean func //////////////////////

typedef void (*ferrum_clean_func_ptr_t)(void *ptr);

typedef struct ferrum_clean_func {

  // free function
  public_ ferrum_clean_func_ptr_t func;
  // ptr list for free that endswith zero
  public_ void **ptr;
  // any data for you
  union {
    int32_t source;
    void *ptr;
  } anydata;

} ferrum_clean_func_t;

#endif
