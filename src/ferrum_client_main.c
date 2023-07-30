#include "ferrum_client.h"
void close_cb(uv_handle_t *handle) {
  ferrum_unused(handle);
  uv_stop(uv_default_loop());
}
void signal_cb(uv_signal_t *handle, int signum) {

  ferrum_unused(signum);
  uv_signal_stop(handle);

  ferrum_log_warn("ctrl+break detected, shutting down\n");
  ferrum_client_t *client = ferrum_cast(handle->data, ferrum_client_t *);
  ferrum_client_destroy(client);
  uv_close(ferrum_cast(handle, uv_handle_t *), close_cb);
}

int main(int argc, char **argv) {
  ferrum_log_set_level_from_env();

  ferrum_client_t *client;
  int32_t result = ferrum_client_new(argc, argv, &client);
  if (result) {
    ferrum_log_fatal("client create failed");
    return FERRUM_ERR;
  }
  result = ferrum_client_start(client);
  if (result) {
    ferrum_log_fatal("client create failed");
    ferrum_client_destroy(client);
    return FERRUM_ERR;
  }

  // capture ctrl+c
  uv_signal_t ctrl_c;
  uv_signal_init(uv_default_loop(), &ctrl_c);
  ctrl_c.data = client;
  uv_signal_start(&ctrl_c, signal_cb, SIGINT);
  // so important for reset connections
  // capture SIGPIPE
  signal(SIGPIPE, SIG_IGN);
  // uv_signal_t sigpipe;
  // uv_signal_init(uv_default_loop(), &sigpipe);
  // uv_signal_start(&sigpipe, signal_ignore_cb, SIGPIPE);
  //////////////////////////////////
  uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  for (int32_t wait = 0; wait < 1000; ++wait)
    uv_run(uv_default_loop(), UV_RUN_ONCE);

  uv_loop_close(uv_default_loop());
}