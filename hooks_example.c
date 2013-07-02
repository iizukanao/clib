#include <stdio.h>
#include "hooks.h"

#define HOOKS_DIR "hooks"

pthread_t hooks_thread;

void on_file_create(char *filename, char *contents) {
  printf("filename: %s\n", filename);
  printf("contents: %s\n", contents);
}

int main() {
  if (clear_hooks(HOOKS_DIR) != 0) {
    fprintf(stderr, "clear_hooks() failed\n");
  }
  start_watching_hooks(&hooks_thread, HOOKS_DIR, on_file_create);
}
