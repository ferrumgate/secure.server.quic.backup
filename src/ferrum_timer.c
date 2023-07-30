#include "ferrum_timer.h"

int32_t ferrum_timer_new(ferrum_timer_t **tmr, ferrum_timer_callback_t callback, void *data) {
  ferrum_new4(ferrum_timer_t, timer);
  timer->callback = callback;
  timer->arg = data;
  int32_t result = uv_timer_init(uv_default_loop(), &timer->timer);
  if (result) {
    ferrum_log_error("timer init failed %s\n", uv_strerror(result));
    ferrum_free(timer);
    return FERRUM_ERR_UV + result;
  }
  timer->timer.data = timer;
  *tmr = timer;

  return FERRUM_SUCCESS;
}

static void on_timer_close(uv_handle_t *handle) {
  uv_timer_t *timer = ferrum_cast(handle, uv_timer_t *);
  if (timer->data)
    ferrum_free(timer->data);
}
int32_t ferrum_timer_destroy(ferrum_timer_t *timer) {
  if (timer) {
    if (timer->is_started) {
      uv_timer_stop(&timer->timer);
    }
    uv_close(ferrum_cast(&timer->timer, uv_handle_t *), on_timer_close);
  }
  return FERRUM_SUCCESS;
}

static void timer_callback(uv_timer_t *handle) {

  ferrum_timer_t *prv = ferrum_cast(handle->data, ferrum_timer_t *);
  if (prv->repeat_ms)
    prv->is_started = FALSE;
  if (prv && prv->callback)
    prv->callback(prv->arg);
}

int32_t ferrum_timer_start(ferrum_timer_t *timer, uint64_t timeout_ms, uint64_t repeat_ms) {
  if (timer->is_started)
    return FERRUM_SUCCESS;
  int32_t result = uv_timer_start(&timer->timer, timer_callback, timeout_ms, repeat_ms);
  if (result) {
    ferrum_log_error("timer start failed %s\n", uv_strerror(result));
    return FERRUM_ERR_UV + result;
  }
  timer->timeout_ms = timeout_ms;
  timer->repeat_ms = repeat_ms;
  timer->is_started = TRUE;
  return FERRUM_SUCCESS;
}
int32_t ferrum_timer_stop(ferrum_timer_t *timer) {
  if (timer->is_started) {
    int32_t result = uv_timer_stop(&timer->timer);
    if (result) {
      ferrum_log_error("timer stop failed %s\n", uv_strerror(result));
      return FERRUM_ERR_UV + result;
    }
    timer->is_started = FALSE;
  }
  return FERRUM_SUCCESS;
}