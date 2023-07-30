#include "ferrum_socket.h"

int32_t ferrum_udp_socket_new(ferrum_udp_socket_t **sck, const ferrum_sockaddr_t *addr) {

  ferrum_new4(ferrum_udp_socket_t, socket);

  int32_t result = uv_udp_init_ex(uv_default_loop(), &socket->handle, UV_UDP_RECVMMSG);
  if (result < 0) {
    ferrum_log_fatal("socket failed:%s\n", uv_strerror(result));
    ferrum_udp_socket_destroy(socket);
    return FERRUM_ERR_UV + result;
  }
  result = uv_udp_bind(&socket->handle, &addr->base, UV_UDP_REUSEADDR);
  if (result < 0) {
    ferrum_log_fatal("socket bind failed:%s\n", uv_strerror(result));
    ferrum_udp_socket_destroy(socket);
    return FERRUM_ERR_UV + result;
  }
  socket->handle.data = socket;
  int32_t tmp = 0;
  result = uv_udp_getsockname(&socket->handle, &socket->local.base, &tmp);
  if (result < 0) {
    ferrum_log_fatal("socket sockname failed:%s\n", uv_strerror(result));
    ferrum_udp_socket_destroy(socket);
    return FERRUM_ERR_UV + result;
  }
  *sck = socket;
  return FERRUM_SUCCESS;
}

int32_t ferrum_udp_socket_set_callbacks(ferrum_udp_socket_t *socket, const ferrum_udp_socket_callback_t *callbacks) {
  socket->on_close = callbacks->on_close;
  socket->callback_data = callbacks->data;
  socket->on_error = callbacks->on_error;
  socket->on_read = callbacks->on_read;
  socket->on_write = callbacks->on_write;
  return FERRUM_SUCCESS;
}

int32_t ferrum_udp_socket_connect(ferrum_udp_socket_t *socket, const ferrum_sockaddr_t *addr) {
  int32_t result = uv_udp_connect(&socket->handle, &addr->base);
  if (result) {
    ferrum_log_error("socket connect failed %s\n", uv_strerror(result));
    return FERRUM_ERR_UV + result;
  }
  socket->dest = *addr;
  socket->is_connected = TRUE;
  return FERRUM_SUCCESS;
}
static void on_recv(uv_udp_t *handle, ssize_t nread, const uv_buf_t *rcvbuf, const struct sockaddr *addr, unsigned flags) {

  ferrum_unused(flags);
  ferrum_udp_socket_t *socket = ferrum_cast(handle->data, ferrum_udp_socket_t *);

  if (socket && !uv_is_closing(ferrum_cast(handle, uv_handle_t *))) {
    if (nread < 0) // error or closed
    {
      if (socket->on_error) {
        ferrum_log_debug("socket error occured %zd\n", nread);
        socket->on_error(socket, socket->callback_data, FERRUM_ERR_UV + nread);
      }
    } else if (socket->on_read && nread) {
      ferrum_log_debug("socket receive nread:%zd buflen:%zu\n", nread, rcvbuf->len);
      socket->on_read(socket, socket->callback_data, addr, ferrum_cast(rcvbuf->base, uint8_t *), nread);
    }
  }
  if (rcvbuf->base && (nread < 0 || !(flags & UV_UDP_MMSG_CHUNK))) {
    ferrum_free(rcvbuf->base);
  }
}

static void on_alloc(uv_handle_t *client, size_t suggested_size, uv_buf_t *buf) {
  ferrum_unused(client);
  if (suggested_size <= 0) {
    ferrum_log_info("socket suggested_size is 0 from \n");
    return;
  }

  buf->base = ferrum_malloc(suggested_size);
  ferrum_if_is_null_then_die(buf->base, "malloc problem\n");

  buf->len = suggested_size;
  ferrum_fill_zero(buf->base, buf->len);
  ferrum_log_debug("malloc socket:%lu %p\n", buf->len, buf->base);
}

int32_t ferrum_udp_socket_start(ferrum_udp_socket_t *socket) {
  int32_t result = uv_udp_recv_start(&socket->handle, on_alloc, on_recv);
  if (result < 0) {
    ferrum_log_error("socket could not started %s\n", uv_strerror(result));
    return FERRUM_ERR_UV + result;
  }
  return FERRUM_SUCCESS;
}
int32_t ferrum_udp_socket_stop(ferrum_udp_socket_t *socket) {
  int32_t result = uv_udp_recv_stop(&socket->handle);
  if (result < 0) {
    ferrum_log_error("socket stop failed %s\n", uv_strerror(result));
    return FERRUM_ERR_UV + result;
  }
  return FERRUM_SUCCESS;
}

static void on_send(uv_udp_send_t *req, int status) {

  ferrum_log_debug("socket on send called and status:%d\n", status);
  ferrum_clean_func_t *clean_func = ferrum_cast(req->data, ferrum_clean_func_t *);
  void *source = (clean_func && clean_func->anydata.ptr) ? clean_func->anydata.ptr : NULL;

  if (req->handle && !uv_is_closing(ferrum_cast(req->handle, uv_handle_t *)) && req->handle->data) {

    ferrum_udp_socket_t *socket = ferrum_cast(req->handle->data, ferrum_udp_socket_t *);
    if (status >= 0) {
      if (socket->on_write)
        socket->on_write(socket, socket->callback_data, source);
    } else {
      if (socket->on_error)
        socket->on_error(socket, socket->callback_data, FERRUM_ERR_UV + status);
    }
  }

  if (clean_func) {
    if (clean_func->func && clean_func->ptr) {
      void **start = clean_func->ptr;
      while (*start) {
        clean_func->func(*start);
        start++;
      }
    }
    ferrum_free(clean_func);
  }
  ferrum_free(req);
}

int32_t ferrum_udp_socket_write(ferrum_udp_socket_t *socket, const ferrum_sockaddr_t *dst_addr, ferrum_buf_t buffers[], size_t buffers_count, ferrum_clean_func_t *clean_func) {

  int32_t result;
  if (uv_is_closing(ferrum_cast(&socket->handle, uv_handle_t *))) {
    return FERRUM_ERR;
  }
  ferrum_new4(uv_udp_send_t, request);
  uv_buf_t *bufs = ferrum_malloc(sizeof(uv_buf_t) * (buffers_count));
  for (size_t i = 0; i < buffers_count; ++i) {
    bufs[i] = uv_buf_init(ferrum_cast(buffers[i].buf, char *), buffers[i].len);
  }
  request->data = clean_func;

  result = uv_udp_send(request, &socket->handle, bufs, buffers_count, socket->is_connected ? NULL : &dst_addr->base, on_send);

  if (result < 0) {

    ferrum_log_info("sending data to server failed\n");
    ferrum_free(bufs);
    ferrum_free(request->data);
    ferrum_free(request);
    return FERRUM_ERR_UV + result;
  }
  ferrum_free(bufs);
  ferrum_log_debug("data sended len:%zu to server\n", buffers[0].len);
  return FERRUM_SUCCESS;
}

static void on_close(uv_handle_t *handle) {
  if (handle)
    if (handle->data && uv_is_closing(handle)) {
      ferrum_udp_socket_t *socket = ferrum_cast(handle->data, ferrum_udp_socket_t *);
      if (socket->on_close)
        socket->on_close(socket, socket->callback_data);
      if (socket)
        ferrum_free(socket);
    }
}

int32_t ferrum_udp_socket_destroy(ferrum_udp_socket_t *socket) {
  if (socket) {
    ferrum_log_debug("closing socket\n");
    uv_close(ferrum_cast(&socket->handle, uv_handle_t *), on_close);
  }
  return FERRUM_SUCCESS;
}