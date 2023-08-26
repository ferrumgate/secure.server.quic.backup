#include "./ferrum_resolve.h"
#include "cmocka.h"
#include <unistd.h>
#include <string.h>

#define loop(var, a, x)                       \
  var = a;                                    \
  while (var-- && (x)) {                      \
    usleep(100);                              \
    uv_run(uv_default_loop(), UV_RUN_NOWAIT); \
  }

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

static void resolve_sync_google_com(void **start) {
  ferrum_unused(start);
  ferrum_sockaddr_t *addrlist;
  size_t len;
  int32_t result = ferrum_resolve_sync("www.google.com", A, &addrlist, &len);
  assert_int_equal(result, FERRUM_SUCCESS);
  assert_int_not_equal(len, 0);
  assert_ptr_not_equal(addrlist, NULL);
  if (addrlist)
    ferrum_free(addrlist);

  result = ferrum_resolve_sync("www.yahoo.com", A, &addrlist, &len);
  assert_int_equal(result, FERRUM_SUCCESS);
  assert_int_not_equal(len, 0);
  assert_ptr_not_equal(addrlist, NULL);
  if (addrlist)
    ferrum_free(addrlist);

  result = ferrum_resolve_sync("www.ya2hoo2.com", A, &addrlist, &len);
  assert_int_equal(result, FERRUM_ERR_RESOLV);
  assert_int_equal(len, 0);
  assert_ptr_equal(addrlist, NULL);
  if (addrlist)
    ferrum_free(addrlist);

  result = ferrum_resolve_sync("ferrumgate.com", AAAA, &addrlist, &len);
  assert_int_equal(result, FERRUM_ERR_RESOLV);
  assert_int_equal(len, 0);
  assert_ptr_equal(addrlist, NULL);
  if (addrlist)
    ferrum_free(addrlist);

  result = ferrum_resolve_sync("1.1.1.1", AAAA, &addrlist, &len);
  assert_int_equal(result, FERRUM_ERR_RESOLV);
  assert_int_equal(len, 0);
  assert_ptr_equal(addrlist, NULL);
  if (addrlist)
    ferrum_free(addrlist);

  result = ferrum_resolve_sync("1.1.1.1", A, &addrlist, &len);
  assert_int_equal(result, FERRUM_SUCCESS);
  assert_int_equal(len, 1);
  assert_ptr_not_equal(addrlist, NULL);
  if (addrlist)
    ferrum_free(addrlist);
}

int test_ferrum_resolve(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(resolve_sync_google_com),

  };
  return cmocka_run_group_tests(tests, setup, teardown);
}
