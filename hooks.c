#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <pthread.h>
#include <signal.h>

#include "hooks.h"

#define NUM_EVENT_BUF 10
#define EVENT_NAME_BUF_LEN 32

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN  ( NUM_EVENT_BUF * ( EVENT_SIZE + EVENT_NAME_BUF_LEN ) )

typedef struct watch_target {
  char *dir;
  void (*callback)(char *);
} watch_target;

static int keep_watching = 1;
static pthread_t *watcher_thread;

void *watch_for_file_creation(watch_target *target) {
  int length, i;
  int fd;
  int wd;
  int dir_strlen;
  char buffer[EVENT_BUF_LEN];
  char *dir = target->dir;
  void (*callback)(char *) = target->callback;

  free(target);
  dir_strlen = strlen(dir);

  fd = inotify_init();
  if (fd < 0) {
    perror("inotify_init error");
    exit(1);
  }

  struct stat st;
  int err = stat(dir, &st);
  if (err == -1) {
    if (errno == ENOENT) {
      fprintf(stderr, "Target directory does not exist\n");
    } else {
      perror("stat error");
    }
    exit(1);
  } else {
    if (!S_ISDIR(st.st_mode)) {
      fprintf(stderr, "Hook target is not a directory\n");
      exit(1);
    }
  }

  if (access(dir, R_OK) != 0) {
    perror("Can't access hook target directory");
    exit(1);
  }

  wd = inotify_add_watch(fd, dir, IN_CREATE);

  while (keep_watching) {
    length = read(fd, buffer, EVENT_BUF_LEN);
    if (length < 0) {
      perror("inotify read error");
      break;
    }

    i = 0;
    while (i < length) {
      struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
      if (event->len) {
        if (event->mask & IN_CREATE) {
          if (!(event->mask & IN_ISDIR)) { // file
            callback(event->name);
            int path_len = dir_strlen + strlen(event->name) + 2;
            char *path = malloc(path_len);
            snprintf(path, path_len, "%s/%s", dir, event->name);
            if (path == NULL) {
              perror("malloc failed");
            } else {
              if (unlink(path) != 0) {
                perror("unlink failed");
              }
              free(path);
            }
          }
        }
      }
      i += EVENT_SIZE + event->len;
    }
  }

  inotify_rm_watch(fd, wd);
  close(fd);
  pthread_exit(0);
}

void start_watching_hooks(pthread_t *thread, char *dir, void (*callback)(char *)) {
  watch_target *target = malloc(sizeof(watch_target));
  target->dir = dir;
  target->callback = callback;
  pthread_create(thread, NULL, (void * (*)(void *))watch_for_file_creation, target);
  watcher_thread = thread;
}

void stop_watching_hooks() {
  keep_watching = 0;
  pthread_kill(*watcher_thread, SIGTERM);
}
