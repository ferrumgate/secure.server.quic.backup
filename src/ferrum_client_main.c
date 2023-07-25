#include "ferrum_client.h"

int main(int argc, char **argv) {
  printf("%d argc\n", argc);
  for (int i = 0; i < argc; ++i)
    printf("%s\n", argv[i]);
}