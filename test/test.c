
extern int test_ferrum_client();
extern int test_ferrum_util();
extern int test_ferrum_resolve();
extern int test_ferrum_timer();
extern int test_ferrum_socket();
int main() {
  test_ferrum_resolve();
  test_ferrum_util();
  test_ferrum_timer();
  test_ferrum_socket();
  test_ferrum_client();

  return 0;
}