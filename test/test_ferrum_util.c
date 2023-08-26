#include "./ferrum_util.h"
#include "cmocka.h"
#include <unistd.h>
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

static void test_ferrum_util_to_int64_t() {
  int result;
  int64_t val;

  result = ferrum_util_to_int64_t("0", &val);
  assert_int_equal(result, FERRUM_SUCCESS);
  assert_int_equal(val, 0);

  result = ferrum_util_to_int64_t("-1", &val);
  assert_int_equal(result, FERRUM_SUCCESS);
  assert_int_equal(val, -1);

  result = ferrum_util_to_int64_t("9223372036854775807", &val);
  assert_int_equal(result, FERRUM_SUCCESS);
  assert_int_equal(val, 9223372036854775807);

  result = ferrum_util_to_int64_t("92233720368547758071", &val);
  assert_int_equal(result, FERRUM_ERR_BAD_ARGUMENT);

  result = ferrum_util_to_int64_t("-9223372036854775808", &val);
  assert_int_equal(result, FERRUM_SUCCESS);
  // assert_int_equal(val, -9223372036854775808);

  result = ferrum_util_to_int64_t("-92233720368547758081", &val);
  assert_int_equal(result, FERRUM_ERR_BAD_ARGUMENT);

  result = ferrum_util_to_int64_t("abc", &val);
  assert_int_equal(result, FERRUM_ERR_BAD_ARGUMENT);
}

static void test_ferrum_util_to_int32_t() {
  int result;
  int32_t val;

  result = ferrum_util_to_int32_t("0", &val);
  assert_int_equal(result, FERRUM_SUCCESS);
  assert_int_equal(val, 0);

  result = ferrum_util_to_int32_t("-1", &val);
  assert_int_equal(result, FERRUM_SUCCESS);
  assert_int_equal(val, -1);

  result = ferrum_util_to_int32_t("2147483647", &val);
  assert_int_equal(result, FERRUM_SUCCESS);
  assert_int_equal(val, 2147483647);

  result = ferrum_util_to_int32_t("9147483647", &val);
  assert_int_equal(result, FERRUM_ERR_BAD_ARGUMENT);

  result = ferrum_util_to_int32_t("-2147483648", &val);
  assert_int_equal(result, FERRUM_SUCCESS);
  assert_int_equal(val, -2147483648);

  result = ferrum_util_to_int32_t("-21474836481", &val);
  assert_int_equal(result, FERRUM_ERR_BAD_ARGUMENT);
}

static void test_ferrum_util_to_uint32_t() {
  int result;
  uint32_t val;

  result = ferrum_util_to_uint32_t("0", &val);
  assert_int_equal(result, FERRUM_SUCCESS);
  assert_int_equal(val, 0);

  result = ferrum_util_to_uint32_t("-1", &val);
  assert_int_equal(result, FERRUM_ERR_BAD_ARGUMENT);

  result = ferrum_util_to_uint32_t("4294967295", &val);
  assert_int_equal(result, FERRUM_SUCCESS);
  assert_int_equal(val, 4294967295);

  result = ferrum_util_to_uint32_t("42949672951", &val);
  assert_int_equal(result, FERRUM_ERR_BAD_ARGUMENT);

  result = ferrum_util_to_uint32_t("-21474836481", &val);
  assert_int_equal(result, FERRUM_ERR_BAD_ARGUMENT);
}

int test_ferrum_util(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_ferrum_util_to_uint32_t),
      cmocka_unit_test(test_ferrum_util_to_int32_t),
      cmocka_unit_test(test_ferrum_util_to_int64_t)

  };
  return cmocka_run_group_tests(tests, setup, teardown);
}
