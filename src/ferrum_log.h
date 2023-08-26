#ifndef __FERRUM_LOG_H__
#define __FERRUM_LOG_H__
#include "ferrum_common.h"
#include "ferrum_util.h"

typedef enum {
  FERRUM_LOG_OFF = 0,
  FERRUM_LOG_FATAL = 1,
  FERRUM_LOG_ERROR = 2,
  FERRUM_LOG_WARN = 3,
  FERRUM_LOG_INFO = 4,
  FERRUM_LOG_DEBUG = 5,
  FERRUM_LOG_ALL = 6
} log_level_t;

#define FERRUM_LOG_LEVEL_STR 16

void ferrum_log_level(log_level_t level);
void ferrum_log_set_level_from_env();

void ferrum_log_info2(const char *file, int32_t line, const char *fmt, ...);
void ferrum_log_debug2(const char *file, int32_t line, const char *fmt, ...);
void ferrum_log_fatal2(const char *file, int32_t line, const char *fmt, ...);
void ferrum_log_error2(const char *file, int32_t line, const char *fmt, ...);
void ferrum_log_warn2(const char *file, int32_t line, const char *fmt, ...);

#define ferrum_log_info(fmt, ...) ferrum_log_info2(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ferrum_log_debug(fmt, ...) ferrum_log_debug2(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ferrum_log_fatal(fmt, ...) ferrum_log_fatal2(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ferrum_log_error(fmt, ...) ferrum_log_error2(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define ferrum_log_warn(fmt, ...) ferrum_log_warn2(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif
