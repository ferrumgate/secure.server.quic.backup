#ifndef __FERRUM_IO_TARGET_H__
#define __FERRUM_IO_TARGET_H__
#include "ferrum_common.h"
#include "ferrum_log.h"

typedef void (*write_t)(uint8_t *buf, size_t len);
typedef void (*on_read_callback_t)(uint8_t *buf, size_t len);
typedef void (*on_write_callback_t)(uint8_t *buf, size_t len);
typedef void (*close_t)();
typedef void (*on_close_t)();

// base abstract class
#define ferrum_io_target_base() \
  write_t write;                \
  on_read_callback_t on_read;   \
  close_t close;                \
  on_close_t on_close

typedef struct ferrum_io_target_buf {
  ferrum_io_target_base();
  char buf[4096];
  size_t buf_len;

} ferrum_io_target_buf_t;

int32_t ferrum_io_target_buf_new(ferrum_io_target_buf_t **target, write_callback_t write);
#endif