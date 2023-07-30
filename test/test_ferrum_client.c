#include "./ferrum_client.h"
#include "cmocka.h"
#include "common.h"

static int setup(void **state) {
  ferrum_unused(state);
  fprintf(stdout, "****  %s ****\n", __FILE__);
  return 0;
}

static int teardown(void **state) {
  ferrum_unused(state);
  uv_loop_close(uv_default_loop());
  return 0;
}

static int test = 0;

static int32_t callback(void *data) {
  ferrum_unused(data);
  test++;
  return test;
}

static void test_ferrum_client_config() {
  ferrum_client_config_t *config;

  // default settings
  char *argv[] = {"test"};
  int32_t result = ferrum_client_config_parse(1, argv, &config);
  assert_int_equal(result, FERRUM_SUCCESS);
  assert_string_equal(config->host, "localhost");
  assert_string_equal(config->port, "8443");
  ferrum_client_config_destroy(config);

  // valid input
  char *argv1[] = {"test", "-h", "test", "-p", "12", "-l", "INFO", "-a", "tu"};
  result = ferrum_client_config_parse(5, argv1, &config);
  assert_int_equal(result, FERRUM_SUCCESS);
  assert_string_equal(config->host, "test");
  assert_string_equal(config->port, "12");
  assert_string_equal(config->loglevel, "info");
  assert_string_equal(config->alpn, "tu");
  ferrum_client_config_destroy(config);

  // invalid input
  char *argv2[] = {"test", "-kh", "-p", "12"};
  result = ferrum_client_config_parse(4, argv2, &config);
  assert_int_equal(result, FERRUM_ERR_BAD_ARGUMENT);
}

static void test_ferrum_client_new(void **start) {
  ferrum_unused(start);
  int32_t counter;
  ferrum_client_t *client;
  int32_t result = ferrum_client_new(0, NULL, &client);
  assert_int_equal(result, FERRUM_SUCCESS);
  ferrum_client_destroy(client);

  loop(counter, 100, TRUE);
}

int test_ferrum_client(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_ferrum_client_config),
      //  cmocka_unit_test(test_ferrum_client_new),

  };
  return cmocka_run_group_tests(tests, setup, teardown);
}
