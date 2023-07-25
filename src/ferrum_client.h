#ifndef __FERRUM_CLIENT_H__
#define __FERRUM_CLIENT_H__

#include <getopt.h>
#include "ferrum_common.h"
#include "ferrum_log.h"
#include "ferrum_util.h"

typedef struct ferrum_client_config {
  char host[FERRUM_HOST_STR_LEN];
  char port[FERRUM_PORT_STR_LEN];
} ferrum_client_config_t;

typedef struct ferrum_client_context {
  lsquic_conn_t *conn;
  lsquic_stream_t *stream;
  struct {
    uint8_t *buf;
    size_t pos;
    size_t len;
    size_t total;
  } read;

} ferrum_client_context_t;

typedef struct ferrum_client {

  struct lsquic_engine_settings quic_settings;
  struct lsquic_engine_api quic_api;
  struct lsquic_engine *quic_engine;
  const char hostname[128];
  SSL_CTX *ssl_ctx;
  ferrum_client_context_t *context;
  ferrum_client_config_t *config;

} ferrum_client_t;

int32_t ferrum_client_config_parse(int argc, char **argv, ferrum_client_config_t **cfg);

int32_t ferrum_client_config_destroy(ferrum_client_config_t *config);

int32_t ferrum_client_new(int argc, char **argv, ferrum_client_t **cl);
int32_t ferrum_client_destroy(ferrum_client_t *client);

#endif