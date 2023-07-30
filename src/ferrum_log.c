#include "ferrum_log.h"

static log_level_t log_level = FERRUM_LOG_ERROR;
void ferrum_log_level(log_level_t level) {
  log_level = level;
}
void ferrum_log_info2(const char *file, int32_t line, const char *fmt, ...) {
  if (log_level >= FERRUM_LOG_INFO) {
    char current_time_str[32] = {0};
    ferrum_unused(current_time_str);
    fprintf(stderr, "[%s] [INFO] %s:%d - ", ferrum_util_time(current_time_str), file, line);
    va_list myargs;
    va_start(myargs, fmt);
    vfprintf(stderr, fmt, myargs);
    va_end(myargs);
  }
}
void ferrum_log_debug2(const char *file, int32_t line, const char *fmt, ...) {
  if (log_level >= FERRUM_LOG_DEBUG) {
    char current_time_str[32] = {0};
    ferrum_unused(current_time_str);
    fprintf(stderr, "[%s] [DEBUG] %s:%d - ", ferrum_util_time(current_time_str), file, line);
    va_list myargs;
    va_start(myargs, fmt);
    vfprintf(stderr, fmt, myargs);
    va_end(myargs);
  }
}
void ferrum_log_warn2(const char *file, int32_t line, const char *fmt, ...) {
  if (log_level >= FERRUM_LOG_WARN) {
    char current_time_str[32] = {0};
    ferrum_unused(current_time_str);
    fprintf(stderr, "[%s] [WARN] %s:%d - ", ferrum_util_time(current_time_str), file, line);
    va_list myargs;
    va_start(myargs, fmt);
    vfprintf(stderr, fmt, myargs);
    va_end(myargs);
  }
}

void ferrum_log_fatal2(const char *file, int32_t line, const char *fmt, ...) {
  if (log_level >= FERRUM_LOG_FATAL) {
    char current_time_str[32] = {0};
    ferrum_unused(current_time_str);
    fprintf(stderr, "[%s] [FATAL] %s:%d - ", ferrum_util_time(current_time_str), file, line);
    va_list myargs;
    va_start(myargs, fmt);
    vfprintf(stderr, fmt, myargs);
    va_end(myargs);
  }
}

void ferrum_log_error2(const char *file, int32_t line, const char *fmt, ...) {
  if (log_level >= FERRUM_LOG_FATAL) {
    char current_time_str[32] = {0};
    ferrum_unused(current_time_str);
    fprintf(stderr, "[%s] [ERROR] %s:%d - ", ferrum_util_time(current_time_str), file, line);
    va_list myargs;
    va_start(myargs, fmt);
    vfprintf(stderr, fmt, myargs);
    va_end(myargs);
  }
}

void ferrum_log_set_level_from_env() {
  char log_level[FERRUM_MAX_ENV_LEN] = {0};
  size_t log_level_size = sizeof(log_level);
  uv_os_getenv("LOG_LEVEL", log_level, &log_level_size);
  log_level_t level = FERRUM_LOG_ERROR;
  if (!strcmp(log_level, "off"))
    level = FERRUM_LOG_OFF;
  if (!strcmp(log_level, "fatal"))
    level = FERRUM_LOG_FATAL;
  if (!strcmp(log_level, "error"))
    level = FERRUM_LOG_ERROR;
  if (!strcmp(log_level, "warn"))
    level = FERRUM_LOG_WARN;
  if (!strcmp(log_level, "info"))
    level = FERRUM_LOG_INFO;
  if (!strcmp(log_level, "debug"))
    level = FERRUM_LOG_DEBUG;
  if (!strcmp(log_level, "all"))
    level = FERRUM_LOG_ALL;
  // set log level
  ferrum_log_level(level);
}