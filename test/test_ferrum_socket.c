#include "./ferrum_socket.h"
#include "./server_client/udpecho.h"
#include "common.h"
#include "cmocka.h"

#define UDPSERVER_PORT "9999"
static int setup(void **state) {
  ferrum_unused(state);
  fprintf(stdout, "****  %s ****\n", __FILE__);
  int32_t result = udp_echo_start(atoi(UDPSERVER_PORT));

  return result;
}

static int teardown(void **state) {
  ferrum_unused(state);
  udp_echo_close();
  uv_loop_close(uv_default_loop());
  return 0;
}

static int32_t flag = 0;
static char read_buffer[65536] = {'\0'};
static const char *testdata = "merhaba";
static int32_t closed = 0;
static void on_closed(ferrum_udp_socket_t *socket, void *data) {
  ferrum_unused(socket);
  ferrum_unused(data);
  closed = 1;
}

static void on_error_occured(ferrum_udp_socket_t *socket, void *data, int error) {
  ferrum_unused(socket);
  ferrum_unused(data);
  ferrum_unused(error);
}
static void on_server_received(ferrum_udp_socket_t *socket, void *data, const struct sockaddr *addr, const uint8_t *buffer, ssize_t len) {
  ferrum_unused(addr);
  ferrum_unused(socket);

  assert_string_equal(data, testdata);
  flag = 1;
  memset(read_buffer, 0, 512);
  memcpy(read_buffer, buffer, len < 65536 ? len : 65536);
}

static void on_server_send(ferrum_udp_socket_t *socket, void *data, void *source) {
  ferrum_unused(data);
  ferrum_unused(socket);
  ferrum_unused(source);

  assert_string_equal(data, testdata);
  flag = 2;
}

static void ferrum_udp_socket_asserver_communication(void **start) {
  ferrum_unused(start);
  ferrum_udp_socket_t *server;
  const char *bind_ip = "0.0.0.0";
  const char *bind_port = "9090";
  ferrum_sockaddr_t bind;
  ferrum_util_ip_port_to_addr(bind_ip, bind_port, &bind);

  const char *localhost_ip = "127.0.0.1";

  ferrum_sockaddr_t localhost;
  ferrum_util_ip_port_to_addr(localhost_ip, bind_port, &localhost);

  const char *dest_ip = "127.0.0.1";
  const char *dest_port = "9999";
  ferrum_sockaddr_t client;
  ferrum_util_ip_port_to_addr(dest_ip, dest_port, &client);
  ferrum_new2(ferrum_udp_socket_callback_t, callbacks);
  callbacks.data = ferrum_cast(testdata, void *);
  callbacks.on_read = on_server_received;
  callbacks.on_write = on_server_send;
  callbacks.on_error = on_error_occured;

  int32_t result = ferrum_udp_socket_new(&server, &bind);
  assert_int_equal(result, 0);
  ferrum_udp_socket_set_callbacks(server, &callbacks);
  ferrum_udp_socket_start(server);
  // check loop
  uv_run(uv_default_loop(), UV_RUN_NOWAIT);

  const char *msg = "hello world";

  flag = 0;
  udp_echo_send2(msg, &localhost.v4);
  // loop again
  uv_run(uv_default_loop(), UV_RUN_NOWAIT);
  // check for received data
  int32_t max_check = 10;
  // loop(max_check,10,!flag)
  while (!flag && max_check) {
    usleep(10000);
    uv_run(uv_default_loop(), UV_RUN_NOWAIT);
    max_check--;
  }
  // ferrum_free(buffer);
  assert_int_not_equal(max_check, 0);
  assert_string_equal(msg, read_buffer);
  flag = 0;
  char *reply = "got it";
  ferrum_buf_t buf = {.buf = ferrum_cast_to_uint8ptr(reply), .len = strlen(reply) + 1};
  ferrum_new4(ferrum_clean_func_t, clean);
  result = ferrum_udp_socket_write(server, &client, &buf, 1, clean);
  assert_int_equal(result, 0);

  // check for received data
  max_check = 10;
  while (!flag && max_check) {
    usleep(10000);
    uv_run(uv_default_loop(), UV_RUN_NOWAIT);
    max_check--;

    result = udp_echo_recv(read_buffer);
    if (result > 0)
      break;
  }
  assert_int_not_equal(max_check, 0);
  assert_string_equal(reply, read_buffer);

  result = ferrum_udp_socket_destroy(server);
  assert_int_equal(result, 0);
  loop(max_check, 1000, TRUE);
}

static void on_dnsclient_error_occured(ferrum_udp_socket_t *socket, void *data, int32_t error) {

  ferrum_unused(socket);
  ferrum_unused(data);
  ferrum_unused(error);
}

static int32_t received_count = 0;
static void on_dnsclient_received(ferrum_udp_socket_t *socket, void *data, const struct sockaddr *addr, const uint8_t *buffer, ssize_t len) {
  ferrum_unused(addr);
  ferrum_unused(socket);
  ferrum_unused(data);
  ferrum_unused(buffer);
  ferrum_unused(len);

  received_count++;
}
static int32_t sended_count = 0;
static void on_dnsclient_send(ferrum_udp_socket_t *socket, void *data, void *source) {
  ferrum_unused(data);
  ferrum_unused(socket);
  ferrum_unused(source);
  ferrum_unused(data);

  sended_count++;
}

/////////////////////// memory tests ///////////////////////////////////////////////

/**
 * @brief create socket, send a packet, then destory socket
 * and test this situation more
 *
 * @param state
 */
static void test_ferrum_udp_socket_check_memory(void **state) {
  // try sending the same dnsd packet
  // to dockerized bind server
  // and check memory of program

  // create and send sockets much

  ferrum_unused(state);
  // read a sample dns packet
  char *testdata;
  size_t datalen;

  int32_t result = ferrum_util_file_read_allbytes("./test/testdata/testpacket1.packet", &testdata, &datalen);
  if (result)
    result = ferrum_util_file_read_allbytes("./testdata/testpacket1.packet", &testdata, &datalen);
  assert_int_equal(datalen, 37);

  const char *dest_ip = "127.0.0.1";
  const char *dest_port = "5555";
  ferrum_sockaddr_t destination;
  ferrum_util_ip_port_to_addr(dest_ip, dest_port, &destination);

  ferrum_sockaddr_t bindaddr;
  ferrum_util_ip_port_to_addr("0.0.0.0", "0", &bindaddr);

  ferrum_new2(ferrum_udp_socket_callback_t, callbacks);
  callbacks.data = NULL;
  callbacks.on_read = on_dnsclient_received;
  callbacks.on_write = on_dnsclient_send;
  callbacks.on_error = on_dnsclient_error_occured;

#define COUNTER 250
  for (int i = 0; i < COUNTER; ++i) {
    ferrum_udp_socket_t *dnsclient;
    result = ferrum_udp_socket_new(&dnsclient, &bindaddr);
    assert_int_equal(result, 0);
    ferrum_udp_socket_set_callbacks(dnsclient, &callbacks);
    ferrum_udp_socket_start(dnsclient);

    sended_count = 0;
    received_count = 0;
    ferrum_new4(ferrum_clean_func_t, clean);
    ferrum_buf_t buf = {.buf = ferrum_cast_to_uint8ptr(testdata), .len = datalen};
    ferrum_udp_socket_write(dnsclient, &destination, &buf, 1, clean);
    int counter = 10000;
    loop(counter, 10000, !sended_count);

    assert_int_equal(sended_count, 1);
    // data sended

    counter = 10000;
    loop(counter, 10000, !received_count);

    assert_int_equal(received_count, 1);
    ferrum_udp_socket_destroy(dnsclient);
    counter = 1000;
    loop(counter, 1000, TRUE);
  }
  ferrum_free(testdata);
}

/**
 * @brief create a socket
 * send lots of packets
 * at the end destory socket
 *
 * @param state
 */
static void test_ferrum_udp_socket_check_memory2(void **state) {
  // try sending the same dnsd packet
  // to dockerized bind server
  // and check memory of program

  // create socket once, send packets mode

  ferrum_unused(state);

  // read a sample dns packet
  char *testdata;
  size_t datalen;

  int32_t result = ferrum_util_file_read_allbytes("./testtestdata/testpacket1.packet", &testdata, &datalen);
  if (result)
    result = ferrum_util_file_read_allbytes("./testdata/testpacket1.packet", &testdata, &datalen);
  assert_int_equal(datalen, 37);

  const char *dest_ip = "127.0.0.1";
  const char *dest_port = "5555";
  ferrum_sockaddr_t destination;
  ferrum_util_ip_port_to_addr(dest_ip, dest_port, &destination);

  ferrum_sockaddr_t bindaddr;
  ferrum_util_ip_port_to_addr("0.0.0.0", "0", &bindaddr);
  ferrum_udp_socket_t *dnsclient;

  ferrum_new2(ferrum_udp_socket_callback_t, callbacks);
  callbacks.data = NULL;
  callbacks.on_read = on_dnsclient_received;
  callbacks.on_write = on_dnsclient_send;
  callbacks.on_error = on_dnsclient_error_occured;

  result = ferrum_udp_socket_new(&dnsclient, &bindaddr);
  assert_int_equal(result, 0);
  ferrum_udp_socket_set_callbacks(dnsclient, &callbacks);
  ferrum_udp_socket_start(dnsclient);
#define COUNTER 250
  for (int i = 0; i < COUNTER; ++i) {

    sended_count = 0;
    received_count = 0;
    ferrum_new4(ferrum_clean_func_t, clean);
    ferrum_buf_t buf = {.buf = ferrum_cast_to_uint8ptr(testdata), .len = datalen};
    ferrum_udp_socket_write(dnsclient, &destination, &buf, 1, clean);

    int counter = 10000;
    loop(counter, 10000, !sended_count);

    assert_int_equal(sended_count, 1);
    // data sended

    counter = 10000;
    loop(counter, 10000, !received_count);

    assert_true(received_count > 0);
  }
  ferrum_udp_socket_destroy(dnsclient);
  int32_t counter = 100;
  loop(counter, 100, TRUE);
  ferrum_free(testdata);
}

/**
 * @brief create a udp server and send packets with hping3
 * hping3 --flood --rand-source --udp -d 25  -p TARGET_PORT TARGET_IP
 * @param state
 */
static void test_ferrum_udp_socket_check_memory3(void **state) {
  // create a udp server
  // and send packets with hping3

  ferrum_unused(state);
  ferrum_sockaddr_t bindaddr;
  ferrum_util_ip_port_to_addr("0.0.0.0", "9595", &bindaddr);
  ferrum_udp_socket_t *dnsclient;

  ferrum_new2(ferrum_udp_socket_callback_t, callbacks);
  callbacks.data = NULL;
  callbacks.on_read = on_dnsclient_received;
  callbacks.on_write = NULL;
  callbacks.on_error = on_dnsclient_error_occured;

  int32_t result = ferrum_udp_socket_new(&dnsclient, &bindaddr);
  assert_int_equal(result, 0);
  ferrum_udp_socket_set_callbacks(dnsclient, &callbacks);
  ferrum_udp_socket_start(dnsclient);
  received_count = 0;
  // istenirse burası ile memory test yapılabilir
#define COUNTER2 250
  for (int i = 0; i < COUNTER2; ++i) {

    sended_count = 0;
    uv_run(uv_default_loop(), UV_RUN_NOWAIT);
  }

  ferrum_udp_socket_destroy(dnsclient);
  int32_t counter = 100;
  loop(counter, 1000, TRUE);
}

int test_ferrum_socket(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(ferrum_udp_socket_asserver_communication),
      // cmocka_unit_test(test_ferrum_udp_socket_check_memory),
      cmocka_unit_test(test_ferrum_udp_socket_check_memory2),
      cmocka_unit_test(test_ferrum_udp_socket_check_memory3)};
  return cmocka_run_group_tests(tests, setup, teardown);
}
