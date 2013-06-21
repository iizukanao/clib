#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "state.h"

void state_set(char *dir, char *name, char *value) {
  FILE *fp;
  char *path;
  int path_len;

  path_len = strlen(dir) + strlen(name) + 2;
  path = malloc(path_len);
  if (path == NULL) {
    perror("Memory allocation of path failed");
    return;
  }
  snprintf(path, path_len, "%s/%s", dir, name);
  fp = fopen(path, "w");
  if (fp == NULL) {
    perror("Can't open state file");
    return;
  }
  fwrite(value, 1, strlen(value), fp);
  fclose(fp);
  free(path);
}

void state_get(char *dir, char *name, char **buf) {
  FILE *fp;
  char *path;
  int path_len;
  int size;

  path_len = strlen(dir) + strlen(name) + 2;
  path = malloc(path_len);
  if (path == NULL) {
    perror("Memory allocation of path failed");
    return;
  }
  snprintf(path, path_len, "%s/%s", dir, name);
  fp = fopen(path, "r");
  if (fp == NULL) {
    perror("Can't open state file");
    return;
  }
  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  *buf = malloc(size);
  if (*buf == NULL) {
    perror("Memory allocation of state buffer failed");
    return;
  }
  fread(*buf, 1, size, fp);
  fclose(fp);
  free(path);
}
