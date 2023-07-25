#include "ferrum_client.h"
extern int optind;

void ferrum_client_context_destroy(ferrum_client_context_t *ctx) {
  if (ctx) {
    ferrum_free(ctx);
  }
}

static lsquic_conn_ctx_t *
client_on_new_conn(void *stream_if_ctx, lsquic_conn_t *conn) {
  ferrum_log_debug("client on new connection\n");
  ferrum_client_t *client = ferrum_cast(stream_if_ctx, ferrum_client_t *);
  ferrum_new4(ferrum_client_context_t, context);
  client->context = context;
  context->conn = conn;
  lsquic_conn_set_ctx(conn, ferrum_cast(client, lsquic_conn_ctx_t *));
  lsquic_conn_make_stream(conn);
  return ferrum_cast(client, lsquic_conn_ctx_t *);
}

static void
client_on_conn_closed(lsquic_conn_t *conn) {
  ferrum_log_debug("client on conn closed\n");
  ferrum_client_t *client = ferrum_cast(lsquic_conn_get_ctx(conn), ferrum_client_t *);
  ferrum_log_warn("connection closed");
  lsquic_conn_set_ctx(conn, NULL);
  ferrum_client_context_destroy(client->context);
}

static lsquic_stream_ctx_t *
client_on_new_stream(void *stream_if_ctx, lsquic_stream_t *stream) {
  ferrum_log_debug("client on new stream\n");
  ferrum_client_t *client = ferrum_cast(stream_if_ctx, ferrum_client_t *);
  client->context->stream = stream;
  return ferrum_cast(client, lsquic_stream_ctx_t *);
}

static void
client_on_read(lsquic_stream_t *stream, lsquic_stream_ctx_t *ctx) {
  ferrum_log_debug("client on read\n");
  char c;
  size_t nr;
  ferrum_client_t *client = ferrum_cast(ctx, ferrum_client_t *);
  ferrum_unused(client);
  nr = lsquic_stream_read(stream, &c, 1);
  if (0 == nr) {
    lsquic_stream_shutdown(stream, 2);
    return;
  }
  printf("%c", c);
  fflush(stdout);
  if ('\n' == c) {
    // event_add(st_h->read_stdin_ev, NULL);
    lsquic_stream_wantread(stream, 0);
  }
}

static void
client_on_write(lsquic_stream_t *stream, lsquic_stream_ctx_t *ctx) {
  /* Here we make an assumption that we can write the whole buffer.
   * Don't do it in a real program.
   */
  ferrum_log_debug("client on write\n");
  ferrum_client_t *client = ferrum_cast(ctx, ferrum_client_t *);
  lsquic_stream_write(stream, client->context->read.buf + client->context->read.pos, client->context->read.len - client->context->read.pos);

  lsquic_stream_flush(stream);
  lsquic_stream_wantwrite(stream, 0);
  lsquic_stream_wantread(stream, 1);
}

static void
client_on_stream_close(lsquic_stream_t *stream, lsquic_stream_ctx_t *st_h) {
  ferrum_log_debug("%s called", __func__);
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
  optind = 1; // reset get opt
  while ((c = getopt(argc, argv, "h:p:")) != -1)
    switch (c) {
    case 'h':
      snprintf(config->host, sizeof(config->host) - 1, "%s", optarg);
      break;
    case 'p':
      snprintf(config->port, sizeof(config->port) - 1, "%s", optarg);
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

int32_t ferrum_client_new(int argc, char **argv, ferrum_client_t **cl) {
  ferrum_unused(argc);
  ferrum_unused(argv);
  *cl = NULL;

#ifdef WIN32
  WSADATA wsd;
  int s = WSAStartup(MAKEWORD(2, 2), &wsd);
  if (s != 0) {
    ferrum_log_error("WSAStartup failed: %d", s);
    return -1;
  }
#endif

#ifdef WIN32
  fprintf(stderr, "%s does not work on Windows, see\n"
                  "https://github.com/litespeedtech/lsquic/issues/219\n",
          argv[0]);
  exit(EXIT_FAILURE);
#endif

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

  client->quic_settings.es_idle_timeout = 180000;
  lsquic_engine_init_settings(&client->quic_settings, 0);

  client->quic_api.ea_stream_if = &ferrum_client_stream_if;
  client->quic_api.ea_stream_if_ctx = client;
  client->quic_api.ea_alpn = "tunnel";
  client->quic_api.ea_get_ssl_ctx = get_ssl_ctx;
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
    return -1;
  }

  SSL_CTX_set_min_proto_version(client->ssl_ctx, TLS1_3_VERSION);
  SSL_CTX_set_max_proto_version(client->ssl_ctx, TLS1_3_VERSION);
  SSL_CTX_set_default_verify_paths(client->ssl_ctx);
  *cl = client;
  return FERRUM_SUCCESS;
}

int32_t ferrum_client_start(ferrum_client_t *client) {
  ferrum_unused(client);
  return FERRUM_SUCCESS;
}

int32_t ferrum_client_stop(ferrum_client_t *client) {
  ferrum_unused(client);
  return FERRUM_SUCCESS;
}

int32_t ferrum_client_destroy(ferrum_client_t *client) {
  ferrum_unused(client);
  if (client->quic_engine) {
    lsquic_engine_cooldown(client->quic_engine);
    lsquic_engine_destroy(client->quic_engine);
  }
  if (client->config) {
    ferrum_client_config_destroy(client->config);
  }

  return FERRUM_SUCCESS;
}