#include "ferrum_util.h"

char *ferrum_util_time(char *str) {
  time_t current_time = time(NULL);
  ctime_r(&current_time, str);
  // remove \n
  str[strlen(str) - 1] = 0;
  return str;
}

int32_t ferrum_util_to_int64_t(char *val, int64_t *to) {
  errno = 0;
  char *endptr;
  int64_t val_int = strtoll(val, &endptr, 10);
  if ((errno == ERANGE) || (errno != 0 && val_int == 0LL) || val == endptr) {
    return FERRUM_ERR_BAD_ARGUMENT;
  }

  *to = val_int;
  return FERRUM_SUCCESS;
}
int32_t ferrum_util_to_int32_t(char *val, int32_t *to) {
  errno = 0;
  char *endptr;
  int64_t val_int = strtoll(val, &endptr, 10);
  if ((errno == ERANGE) || (errno != 0 && val_int == 0L) || val == endptr) {

    return FERRUM_ERR_BAD_ARGUMENT;
  }
  if (val_int > INT32_MAX || val_int < INT32_MIN)
    return FERRUM_ERR_BAD_ARGUMENT;
  *to = val_int;
  return FERRUM_SUCCESS;
}
int32_t ferrum_util_to_int16_t(char *val, int16_t *to) {
  errno = 0;
  char *endptr;
  int64_t val_int = strtoll(val, &endptr, 10);
  if ((errno == ERANGE) || (errno != 0 && val_int == 0L) || val == endptr) {

    return FERRUM_ERR_BAD_ARGUMENT;
  }
  if (val_int > INT16_MAX || val_int < INT16_MIN)
    return FERRUM_ERR_BAD_ARGUMENT;
  *to = val_int;
  return FERRUM_SUCCESS;
}
int32_t ferrum_util_to_size_t(char *val, size_t *to) {
  errno = 0;
  char *endptr;
  int64_t val_int = strtoll(val, &endptr, 10);
  if ((errno == ERANGE) || (errno != 0 && val_int == 0L) || val == endptr) {

    return FERRUM_ERR_BAD_ARGUMENT;
  }
  if (val_int > UINT32_MAX || val_int < 0)
    return FERRUM_ERR_BAD_ARGUMENT;
  *to = val_int;
  return FERRUM_SUCCESS;
}

int32_t ferrum_util_to_uint32_t(char *val, uint32_t *to) {
  errno = 0;
  char *endptr;
  uint64_t val_int = strtoull(val, &endptr, 10);
  if ((errno == ERANGE) || (errno != 0 && val_int == 0L) || val == endptr) {
    return FERRUM_ERR_BAD_ARGUMENT;
  }
  if (val_int > UINT32_MAX)
    return FERRUM_ERR_BAD_ARGUMENT;
  *to = val_int;
  return FERRUM_SUCCESS;
}

int32_t ferrum_util_ip_port_to_addr(const char *ip, const char *port, ferrum_sockaddr_t *sock) {
  ferrum_fill_zero(sock, sizeof(ferrum_sockaddr_t));
  if (uv_ip6_addr(ip, atoi(port), ferrum_cast(&sock->v6, struct sockaddr_in6 *)) < 0) {

    if (uv_ip4_addr(ip, atoi(port), ferrum_cast(&sock->v4, struct sockaddr_in *)) < 0) {

      return FERRUM_ERR_BAD_ARGUMENT;
    }
  }
  return FERRUM_SUCCESS;
}

int32_t ferrum_util_addr_to_ferrum_addr(const struct sockaddr *addr, ferrum_sockaddr_t *sock) {

  if (addr->sa_family == AF_INET) {
    memcpy(&sock->v4, addr, sizeof(struct sockaddr_in));
  }
  if (addr->sa_family == AF_INET6) {
    memcpy(&sock->v6, addr, sizeof(struct sockaddr_in6));
  }
  return FERRUM_SUCCESS;
}

int32_t ferrum_util_file_read_allbytes(const char *file, char **buffer, size_t *len) {

  FILE *fileptr;
  int64_t filelen;
  fileptr = fopen(file, "rb");
  if (!fileptr)
    return FERRUM_ERR_BAD_ARGUMENT;
  fseek(fileptr, 0, SEEK_END);
  filelen = ftell(fileptr);
  rewind(fileptr);
  char *temp = ferrum_malloc(filelen + 1);
  ferrum_if_is_null_then_die(temp, "malloc problem\n");

  ferrum_fill_zero(temp, filelen + 1);
  size_t readed = fread(temp, filelen, 1, fileptr);
  ferrum_unused(readed);
  fclose(fileptr);
  *buffer = temp;
  *len = filelen;
  return FERRUM_SUCCESS;
}

int32_t ferrum_util_addr_to_ip_string(const ferrum_sockaddr_t *addr, char buffer[FERRUM_IP_STR_LEN]) {
  if (addr->base.sa_family == AF_INET) {

    uv_ip4_name(&addr->v4, buffer, 16);
  }
  if (addr->base.sa_family == AF_INET6) {

    uv_ip6_name(&addr->v6, buffer, 45);
  }
  return FERRUM_SUCCESS;
}