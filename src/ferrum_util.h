#ifndef __FERRUM_UTIL_H__
#define __FERRUM_UTIL_H__

#include "ferrum_common.h"
#include "ferrum_log.h"
// gets current time
char *ferrum_util_time(char *str);

int32_t ferrum_util_to_int64_t(char *val, int64_t *to);
int32_t ferrum_util_to_int32_t(char *val, int32_t *to);
int32_t ferrum_util_to_int16_t(char *val, int16_t *to);
int32_t ferrum_util_to_uint32_t(char *val, uint32_t *to);
int32_t ferrum_util_to_size_t(char *val, size_t *to);

/**
 * @brief convert a ip string and port to @rebrick_sockaddr_t
 *
 * @param sock
 * @param ip
 * @param port
 * @return int32_t
 */
int32_t ferrum_util_ip_port_to_addr(const char *ip, const char *port, ferrum_sockaddr_t *sock);
int32_t ferrum_util_addr_to_ferrum_addr(const struct sockaddr *addr, ferrum_sockaddr_t *sock);
int32_t ferrum_util_file_read_allbytes(const char *file, char **buffer, size_t *len);
/**
 * @brief converts @see rebrick_sockaddr_t to ip string
 *
 * @param sock
 * @param buffer
 * @param len
 * @return int32_t
 */
int32_t ferrum_util_addr_to_ip_string(const ferrum_sockaddr_t *addr, char buffer[FERRUM_IP_STR_LEN]);

#endif