#ifndef __FERRUM_CLIENT_H__
#define __FERRUM_CLIENT_H__

#include <getopt.h>
#include "ferrum_common.h"
#include "ferrum_log.h"
#include "ferrum_util.h"
#include "ferrum_socket.h"
#include "ferrum_resolve.h"
#include "ferrum_timer.h"

typedef struct ferrum_client_config {
  char host[FERRUM_HOST_STR_LEN];
  char port[FERRUM_PORT_STR_LEN];
  char loglevel[FERRUM_LOG_LEVEL_STR];
  char alpn[FERRUM_TLS_ALPN_MAX_LEN];
} ferrum_client_config_t;

typedef struct ferrum_client_quic_ctx {
  lsquic_conn_t *conn;
  lsquic_stream_t *stream;
  struct {
    uint8_t *buf;
    size_t len;
    size_t total;
  } read;
  struct {
    uint8_t *buf;
    size_t len;
    size_t total;
  } write;

} ferrum_client_quic_ctx_t;

typedef struct ferrum_client {

  struct lsquic_engine_settings quic_settings;
  struct lsquic_engine_api quic_api;
  struct lsquic_engine *quic_engine;

  char resolved_ip[FERRUM_IP_STR_LEN];
  SSL_CTX *ssl_ctx;
  ferrum_client_quic_ctx_t *quic;
  ferrum_client_config_t *config;
  ferrum_udp_socket_t *socket;

  ferrum_timer_t *quic_check_timer;
  int32_t is_socket_destroying;
  int32_t is_quic_ctx_destroying;

} ferrum_client_t;

int32_t ferrum_client_config_parse(int argc, char **argv, ferrum_client_config_t **cfg);
int32_t ferrum_client_config_destroy(ferrum_client_config_t *config);

int32_t ferrum_client_new(int argc, char **argv, ferrum_client_t **cl);
int32_t ferrum_client_destroy(ferrum_client_t *client);
int32_t ferrum_client_start(ferrum_client_t *client);
int32_t ferrum_client_stop(ferrum_client_t *client);

#endif