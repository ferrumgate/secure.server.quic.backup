#ifndef __FERRUM_SOCKET_H__
#define __FERRUM_SOCKET_H__

#include "ferrum_common.h"
#include "ferrum_util.h"
#include "ferrum_log.h"
struct ferrum_udp_socket;

typedef void (*on_read_callback_t)(struct ferrum_udp_socket *socket, void *callback_data, const struct sockaddr *addr, const uint8_t *buffer, ssize_t len);
typedef void (*on_write_callback_t)(struct ferrum_udp_socket *socket, void *callback_data, void *source);
typedef void (*on_error_callback_t)(struct ferrum_udp_socket *socket, void *callback_data, int error);
typedef void (*on_close_callback_t)(struct ferrum_udp_socket *socket, void *callback_data);

typedef struct ferrum_udp_socket_callback {
  void *data;
  on_close_callback_t on_close;
  on_error_callback_t on_error;
  on_write_callback_t on_write;
  on_read_callback_t on_read;
} ferrum_udp_socket_callback_t;

typedef struct ferrum_udp_socket {
  uv_udp_t handle;
  on_close_callback_t on_close;
  on_error_callback_t on_error;
  on_write_callback_t on_write;
  on_read_callback_t on_read;
  void *callback_data;
  int32_t is_connected;
} ferrum_udp_socket_t;

typedef struct ferrum_buf {
  uint8_t *buf;
  size_t len;
} ferrum_buf_t;

int32_t ferrum_udp_socket_new(ferrum_udp_socket_t **socket, const ferrum_sockaddr_t *bind_addr);
int32_t ferrum_udp_socket_set_callbacks(ferrum_udp_socket_t *socket, const ferrum_udp_socket_callback_t *callbacks);
int32_t ferrum_udp_socket_connect(ferrum_udp_socket_t *socket, const ferrum_sockaddr_t *addr);
int32_t ferrum_udp_socket_start(ferrum_udp_socket_t *socket);
int32_t ferrum_udp_socket_stop(ferrum_udp_socket_t *socket);
int32_t ferrum_udp_socket_write(ferrum_udp_socket_t *socket, const ferrum_sockaddr_t *dst_addr, ferrum_buf_t *buffer, ferrum_clean_func_t *clean_func);
int32_t ferrum_udp_socket_destroy(ferrum_udp_socket_t *socket);

#endif