#include "ferrum_client.h"
extern int optind;

static int32_t quic_process_connections(void *data);

void ferrum_client_quic_ctx_destroy(ferrum_client_quic_ctx_t *ctx) {
  if (ctx) {
    if (ctx->read.buf)
      ferrum_free(ctx->read.buf);
    if (ctx->write.buf)
      ferrum_free(ctx->write.buf);
    ferrum_free(ctx);
  }
}

static lsquic_conn_ctx_t *
client_on_new_conn(void *stream_if_ctx, lsquic_conn_t *conn) {
  ferrum_log_debug("quic client on new connection\n");
  ferrum_client_t *client = ferrum_cast(stream_if_ctx, ferrum_client_t *);
  ferrum_new4(ferrum_client_quic_t, quic);
  client->quic = quic;
  quic->conn = conn;
  lsquic_conn_set_ctx(conn, ferrum_cast(client, lsquic_conn_ctx_t *));
  lsquic_conn_make_stream(conn);
  return ferrum_cast(client, lsquic_conn_ctx_t *);
}

static void
client_on_conn_closed(lsquic_conn_t *conn) {
  ferrum_client_t *client = ferrum_cast(lsquic_conn_get_ctx(conn), ferrum_client_t *);
  ferrum_log_warn("quic connection closed\n");
  lsquic_conn_set_ctx(conn, NULL);
  // TODO
  ferrum_client_quic_destroy(client->quic);
}

static lsquic_stream_ctx_t *
client_on_new_stream(void *stream_if_ctx, lsquic_stream_t *stream) {
  ferrum_log_debug("quic client on new stream\n");
  ferrum_client_t *client = ferrum_cast(stream_if_ctx, ferrum_client_t *);
  client->quic->stream = stream;
  return ferrum_cast(client, lsquic_stream_ctx_t *);
}

static void
client_on_read(lsquic_stream_t *stream, lsquic_stream_ctx_t *ctx) {
  ferrum_log_debug("quic client stream on read\n");
  char c;
  size_t nr;
  ferrum_client_t *client = ferrum_cast(ctx, ferrum_client_t *);
  ferrum_unused(client);
  client->quic->read.len = 0;
  nr = lsquic_stream_read(stream, client->quic->read.buf, client->quic->read.total);
  if (0 == nr) {
    ferrum_log_warn("stream closed\n");
    lsquic_stream_shutdown(stream, 2);
    client->is_quic_ctx_destroying = TRUE;
    return;
  } else if (nr < 0) {
    ferrum_log_error("quic client stream read error\n");
    lsquic_stream_shutdown(stream, 2);
    client->is_quic_ctx_destroying = TRUE;
    return;
  } else
    client->quic->read.len = nr;
}

static void
client_on_write(lsquic_stream_t *stream, lsquic_stream_ctx_t *ctx) {
  /* Here we make an assumption that we can write the whole buffer.
   * Don't do it in a real program.
   */
  ferrum_log_debug("quic client stream on write\n");
  ferrum_client_t *client = ferrum_cast(ctx, ferrum_client_t *);
  ssize_t written = lsquic_stream_write(stream, client->quic->write.buf, client->quic->write.len);

  lsquic_stream_flush(stream);
  /* lsquic_stream_wantwrite(stream, 0);
   lsquic_stream_wantread(stream, 1); */
}

static void
client_on_stream_close(lsquic_stream_t *stream, lsquic_stream_ctx_t *st_h) {
  ferrum_log_debug("client on stream closed\n");
  ferrum_unused(st_h);
  lsquic_conn_close(lsquic_stream_conn(stream));
}

const struct lsquic_stream_if ferrum_client_stream_if = {
    .on_new_conn = client_on_new_conn,
    .on_conn_closed = client_on_conn_closed,
    .on_new_stream = client_on_new_stream,
    .on_read = client_on_read,
    .on_write = client_on_write,
    .on_close = client_on_stream_close,
};

static SSL_CTX *
get_ssl_ctx(void *peer_ctx, const struct sockaddr *unused) {
  ferrum_unused(unused);
  const ferrum_client_t *const client = peer_ctx;
  return client->ssl_ctx;
}

static void print_usage() {
  fprintf(stdout, "\n");
  fprintf(stdout, "  -h host\n");
  fprintf(stdout, "  -p port\n");
}

int32_t ferrum_client_config_parse(int argc, char **argv, ferrum_client_config_t **cfg) {

  int c;

  ferrum_new4(ferrum_client_config_t, config);
  // default values
  snprintf(config->host, sizeof(config->host) - 1, "localhost");
  snprintf(config->port, sizeof(config->port) - 1, "8443");
  snprintf(config->loglevel, sizeof(config->loglevel), "info");
  snprintf(config->alpn, sizeof(config->alpn), "tunnel");
  optind = 1; // reset get opt
  while ((c = getopt(argc, argv, "h:p:l:a:")) != -1)
    switch (c) {
    case 'h':
      snprintf(config->host, sizeof(config->host) - 1, "%s", optarg);
      break;
    case 'p':
      snprintf(config->port, sizeof(config->port) - 1, "%s", optarg);
      break;
    case 'l':
      char *arg = optarg;
      while (*arg) {
        *arg = tolower(*arg);
        arg++;
      }
      snprintf(config->loglevel, sizeof(config->loglevel) - 1, "%s", optarg);
      break;
    case 'a':
      snprintf(config->alpn, sizeof(config->alpn) - 1, "%s", optarg);
      break;
    case '?':
      fprintf(stderr,
              "Unknown option character %c'.\n",
              optopt);
      ferrum_free(config);
      return FERRUM_ERR_BAD_ARGUMENT;
    default:
      ferrum_free(config);
      print_usage();
      return FERRUM_ERR_BAD_ARGUMENT;
    }
  *cfg = config;
  return FERRUM_SUCCESS;
}

int32_t ferrum_client_config_destroy(ferrum_client_config_t *config) {
  if (config) {
    ferrum_free(config);
  }
  return FERRUM_SUCCESS;
}

static void on_socket_error(ferrum_udp_socket_t *socket, void *data, int32_t error) {

  ferrum_unused(socket);
  ferrum_unused(data);
  ferrum_unused(error);
  ferrum_log_error("socket error %d\n", error);
}

static void on_socket_read(ferrum_udp_socket_t *socket, void *data, const struct sockaddr *addr, const uint8_t *buffer, ssize_t len) {
  ferrum_unused(addr);
  ferrum_unused(socket);
  ferrum_unused(data);
  ferrum_unused(buffer);
  ferrum_unused(len);
  ferrum_log_debug("socket readed data\n");
  ferrum_client_t *client = ferrum_cast(data, ferrum_client_t *);
  int32_t result = lsquic_engine_packet_in(client->quic_engine, buffer, len, &socket->local.base, &socket->dest.base, client, 0);
  if (result != -1)
    quic_process_connections(client);
}

static void on_socket_write(ferrum_udp_socket_t *socket, void *data, void *source) {
  ferrum_unused(data);
  ferrum_unused(socket);
  ferrum_unused(source);
  ferrum_unused(data);
  ferrum_log_debug("socket written\n");
}
static void on_socket_close(struct ferrum_udp_socket *socket, void *data) {
  ferrum_unused(socket);
  ferrum_unused(data);
  ferrum_log_debug("socket closed\n");
  ferrum_client_t *client = ferrum_cast(data, ferrum_client_t *);
  ferrum_unused(client);
}
void free_data(void *data) {
  if (data)
    ferrum_free(data);
}

int quic_send_packets_out(void *ctx, const struct lsquic_out_spec *specs,
                          unsigned count) {
  ferrum_unused(ctx);
  ferrum_unused(specs);
  ferrum_unused(count);

  if (!count)
    return FERRUM_SUCCESS;
  ferrum_client_t *client = ferrum_cast(ctx, ferrum_client_t *);
  uint32_t n = 0;
  uint32_t total = 0;

  while (n < count) {
    total += specs[n].iovlen;
    n++;
  }
  ferrum_buf_t *bufs = ferrum_new_array(ferrum_buf_t, total);
  ferrum_new4(ferrum_clean_func_t, clean);
  clean->func = free_data;
  clean->ptr = ferrum_new_array(void *, total + 1);
  ferrum_fill_zero(clean->ptr, sizeof(void *) * (total + 1));
  n = 0;
  uint32_t index = 0;
  size_t total_bytes = 0;
  while (n < count) {
    for (size_t i = 0; i < specs[n].iovlen; i++) {
      bufs[index].buf = ferrum_malloc(specs[n].iov[i].iov_len); // specs[n].iov[i].iov_base;
      ferrum_if_is_null_then_die(bufs[index].buf, "malloc problem\n");
      memcpy(bufs[index].buf, specs[n].iov[i].iov_base, specs[n].iov[i].iov_len);
      bufs[index].len = specs[n].iov[i].iov_len;
      clean->ptr[index] = bufs[index].buf;
      total_bytes += specs[n].iov[i].iov_len;
      index++;
    }
    n++;
  }

  int32_t result = ferrum_udp_socket_write(client->socket, NULL, bufs, total, clean);
  if (result) {
    ferrum_log_error("sending failed");
    void **ptr = clean->ptr;
    while (*ptr) {
      ferrum_free(*ptr);
      ptr++;
    }

    ferrum_free(clean);
    ferrum_free(bufs);
    return result;
  }
  return FERRUM_SUCCESS + count;
}

static int
lquic_log_callback(void *ctx, const char *buf, size_t len) {
  ferrum_unused(ctx);
  if (len)
    fwrite(buf, len, 1, stderr);
  return 0;
}

static int
lquic_log_callback_null(void *ctx, const char *buf, size_t len) {
  ferrum_unused(ctx);
  ferrum_unused(buf);
  ferrum_unused(len);
  return 0;
}

static int32_t quic_process_connections(void *data) {
  ferrum_client_t *client = ferrum_cast(data, ferrum_client_t *);
  int diff;
  struct timeval timeout;
  ferrum_log_debug("proces connections\n");
  lsquic_engine_process_conns(client->quic_engine);

  if (lsquic_engine_earliest_adv_tick(client->quic_engine, &diff)) {
    if (diff < 0 || (unsigned)diff < client->quic_settings.es_clock_granularity) {
      timeout.tv_sec = 0;
      timeout.tv_usec = client->quic_settings.es_clock_granularity;
    } else {
      timeout.tv_sec = (unsigned)diff / 1000000;
      timeout.tv_usec = (unsigned)diff % 1000000;
    }
    int32_t ms = timeout.tv_sec * 1000 + (timeout.tv_usec + 500) / 1000;
    if (client->quic_check_timer->is_started) {
      ferrum_log_debug("stoping quic process timer\n");
      ferrum_timer_stop(client->quic_check_timer);
    }
    ferrum_log_debug("starting quic process timer %d\n", ms);
    int32_t result = ferrum_timer_start(client->quic_check_timer, ms, 0);
    if (result) {
      ferrum_log_error("restart quic process connections failed\n");
    }
  }
  return FERRUM_SUCCESS;
}

static struct ssl_ctx_st *
no_cert(void *cert_lu_ctx, const struct sockaddr *sa_UNUSED, const char *sni) {
  ferrum_unused(cert_lu_ctx);
  ferrum_unused(sa_UNUSED);
  ferrum_unused(sni);
  return NULL;
}

int32_t ferrum_client_new(int argc, char **argv, ferrum_client_t **cl) {
  ferrum_unused(argc);
  ferrum_unused(argv);
  *cl = NULL;

  int32_t result = lsquic_global_init(LSQUIC_GLOBAL_CLIENT);
  if (result) {
    ferrum_log_error("lsquic init failed\n");
    return FERRUM_ERR;
  }

  ferrum_client_config_t *config;
  result = ferrum_client_config_parse(argc, argv, &config);
  if (result) {
    ferrum_log_error("config could not parsed\n");
    return result;
  }
  ferrum_new4(ferrum_client_t, client);
  client->config = config;

  result = ferrum_timer_new(&client->quic_check_timer, quic_process_connections, client);
  if (result) {
    ferrum_log_error("config could not parsed\n");
    ferrum_client_destroy(client);
    return result;
  }

  /// log callbacks for lsquic

  static const struct lsquic_logger_if logger_if = {
      .log_buf = lquic_log_callback,
  };
  static const struct lsquic_logger_if logger_if_null = {
      .log_buf = lquic_log_callback_null,
  };
  if (!strcmp(config->loglevel, "debug"))
    lsquic_logger_init(&logger_if, NULL, LLTS_HHMMSSUS);
  else
    lsquic_logger_init(&logger_if_null, NULL, LLTS_HHMMSSUS);
  lsquic_set_log_level(config->loglevel);
  char event[32] = {0};
  snprintf(event, sizeof(event) - 1, "=%s", config->loglevel);
  lsquic_logger_lopt(event);

  // socket operations
  ferrum_sockaddr_t source_addr;
  result = ferrum_util_ip_port_to_addr("::", "0", &source_addr);
  if (result) {
    ferrum_log_error("bind address failed\n");
    ferrum_client_destroy(client);
    return result;
  }
  // resolve host to ip
  ferrum_sockaddr_t *dest_ips;
  size_t dest_ips_len = 0;
  result = ferrum_resolve_sync(config->host, A, &dest_ips, &dest_ips_len);
  if (result) {
    ferrum_log_error("resolve host failed %s\n", config->host);
    ferrum_client_destroy(client);
    return result;
  }

  if (!dest_ips_len) {
    ferrum_log_error("resolve host failed not found %s\n", config->host);
    ferrum_client_destroy(client);
    return result;
  }
  result = ferrum_util_addr_to_ip_string(&dest_ips[0], client->resolved_ip);
  if (result) {
    ferrum_log_error("resolve host failed %s\n", config->host);
    ferrum_client_destroy(client);
    ferrum_free(dest_ips);
    return result;
  }
  ferrum_free(dest_ips);

  // create socket

  ferrum_udp_socket_t *socket;
  result = ferrum_udp_socket_new(&socket, &source_addr);
  if (result) {
    ferrum_log_error("bind address failed\n");
    ferrum_client_destroy(client);
    return result;
  }
  ferrum_udp_socket_callback_t callbacks = {
      .data = client,
      .on_close = on_socket_close,
      .on_error = on_socket_error,
      .on_read = on_socket_read,
      .on_write = on_socket_write,
  };
  ferrum_udp_socket_set_callbacks(socket, &callbacks);
  client->socket = socket;

  client->quic_settings.es_idle_timeout = 180000;
  lsquic_engine_init_settings(&client->quic_settings, 0);

  client->quic_api.ea_packets_out = &quic_send_packets_out;
  client->quic_api.ea_packets_out_ctx = client;

  client->quic_api.ea_stream_if = &ferrum_client_stream_if;
  client->quic_api.ea_stream_if_ctx = client;
  client->quic_api.ea_alpn = config->alpn;
  client->quic_api.ea_get_ssl_ctx = get_ssl_ctx;
  client->quic_api.ea_lookup_cert = no_cert;
  client->quic_api.ea_pmi = NULL;
  client->quic_api.ea_pmi_ctx = NULL;

  char err_buf[64] = {0};
  if (0 != lsquic_engine_check_settings(&client->quic_settings,
                                        0, err_buf, sizeof(err_buf))) {
    ferrum_log_fatal("error in settings: %s\n", err_buf);
    ferrum_client_destroy(client);
    return FERRUM_ERR;
  }

  client->quic_engine = lsquic_engine_new(0, &client->quic_api);
  if (!client->quic_engine) {
    ferrum_log_fatal("engine create failed\n");
    ferrum_client_destroy(client);
    return FERRUM_ERR;
  }

  unsigned char ticket_keys[48];
  ferrum_unused(ticket_keys);
  client->ssl_ctx = SSL_CTX_new(TLS_method());
  if (!client->ssl_ctx) {
    ferrum_log_fatal("cannot allocate SSL context");
    ferrum_client_destroy(client);
    return FERRUM_ERR;
  }

  SSL_CTX_set_min_proto_version(client->ssl_ctx, TLS1_3_VERSION);
  SSL_CTX_set_max_proto_version(client->ssl_ctx, TLS1_3_VERSION);
  SSL_CTX_set_default_verify_paths(client->ssl_ctx);
  SSL_CTX_set_session_cache_mode(client->ssl_ctx,
                                 SSL_SESS_CACHE_CLIENT);
  SSL_CTX_set_early_data_enabled(client->ssl_ctx, 1);
  /*   memset(ticket_keys, 0, sizeof(ticket_keys));
    if (1 != SSL_CTX_set_tlsext_ticket_keys(client->ssl_ctx,
                                            ticket_keys, sizeof(ticket_keys))) {
      ferrum_log_error("SSL_CTX_set_tlsext_ticket_keys failed");
      return -1;
    } */
  *cl = client;
  return FERRUM_SUCCESS;
}

int32_t ferrum_client_start(ferrum_client_t *client) {
  ferrum_unused(client);
  int32_t result = ferrum_udp_socket_start(client->socket);
  if (result) {
    ferrum_log_error("socket start failed\n");
    return FERRUM_ERR;
  }
  ferrum_sockaddr_t dest;
  result = ferrum_util_ip_port_to_addr(client->resolved_ip, client->config->port, &dest);
  if (result) {
    ferrum_log_error("ip port to addr failed %s:%s failed\n", client->resolved_ip, client->config->port);
    return FERRUM_ERR;
  }
  result = ferrum_udp_socket_connect(client->socket, &dest);
  if (result) {
    ferrum_log_error("ip port to addr failed %s:%s failed\n", client->resolved_ip, client->config->port);
    return FERRUM_ERR;
  }

  lsquic_engine_connect(client->quic_engine, N_LSQVER, &client->socket->local.base,
                        &client->socket->dest.base, client, ferrum_cast(client, lsquic_conn_ctx_t *),
                        NULL, 0, NULL, 0, NULL, 0);
  quic_process_connections(client);
  return FERRUM_SUCCESS;
}

int32_t ferrum_client_stop(ferrum_client_t *client) {
  ferrum_unused(client);
  int32_t result = ferrum_udp_socket_stop(client->socket);
  if (result) {
    ferrum_log_error("socket stop failed\n");
    return FERRUM_ERR;
  }
  return FERRUM_SUCCESS;
}

int32_t ferrum_client_destroy(ferrum_client_t *client) {
  ferrum_unused(client);
  if (client->quic->stream)
    lsquic_stream_close(client->quic->stream);
  if (client->quic->conn)
    lsquic_conn_close(client->quic->conn);
  if (client->quic_engine) {
    lsquic_engine_cooldown(client->quic_engine);
    quic_process_connections(client);

    lsquic_engine_destroy(client->quic_engine);
  }
  if (client->quic_check_timer) {
    if (client->quic_check_timer->is_started)
      ferrum_timer_stop(client->quic_check_timer);
    ferrum_timer_destroy(client->quic_check_timer);
  }
  if (client->socket) {
    ferrum_udp_socket_destroy(client->socket);
  }
  if (client->config) {
    ferrum_client_config_destroy(client->config);
  }
  if (client->ssl_ctx) {
    SSL_CTX_free(client->ssl_ctx);
  }

  return FERRUM_SUCCESS;
}