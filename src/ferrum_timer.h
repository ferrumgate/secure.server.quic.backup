#ifndef __FERRUM_TIMER_H__
#define __FERRUM_TIMER_H__
#include "ferrum_common.h"
#include "ferrum_log.h"

typedef int32_t (*ferrum_timer_callback_t)(void *data);

typedef struct ferrum_timer {
  private_ ferrum_timer_callback_t callback;
  private_ void *arg;
  private_ uv_timer_t timer;
  public_ readonly_ int32_t is_started;
  public_ readonly_ uint64_t timeout_ms;
  public_ readonly_ uint64_t repeat_ms;
} ferrum_timer_t;
int32_t ferrum_timer_new(ferrum_timer_t **timer, ferrum_timer_callback_t callback, void *arg);
int32_t ferrum_timer_destroy(ferrum_timer_t *timer);
int32_t ferrum_timer_start(ferrum_timer_t *timer, uint64_t timeout_ms, uint64_t repeat_ms);
int32_t ferrum_timer_stop(ferrum_timer_t *timer);
#endif