#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAXBUF 5
#define ITEMS 20

struct shared_buffer {
  int buf[MAXBUF];
  int in;
  int out;
  int count;
  int capacity;
};

static void
print_wait(int sem, const char *who, const char *why)
{
  sem_wait(sem);
  printf("%s waiting: %s\n", who, why);
  sem_post(sem);
}

int
main(int argc, char **argv)
{
  int capacity = 5;
  if (argc > 1) {
    capacity = atoi(argv[1]);
    if (capacity < 1 || capacity > MAXBUF) {
      printf("Usage: prodcons [buffer_size 1-5]\n");
      exit(1);
    }
  }

  struct shared_buffer *s = (struct shared_buffer *)shm_get();
  if (s == (void *)-1) {
    printf("shm_get failed\n");
    exit(1);
  }
  s->in = 0;
  s->out = 0;
  s->count = 0;
  s->capacity = capacity;

  int empty = sem_create(capacity);
  int full = sem_create(0);
  int mutex = sem_create(1);
  int print = sem_create(1);
  if (empty < 0 || full < 0 || mutex < 0 || print < 0) {
    printf("semaphore creation failed\n");
    exit(1);
  }

  int pid = fork();
  if (pid < 0) {
    printf("fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    // Consumer.
    for (int i = 0; i < ITEMS; i++) {
      sem_wait(mutex);
      int is_empty = (s->count == 0);
      sem_post(mutex);
      if (is_empty)
        print_wait(print, "Consumer", "buffer empty");

      sem_wait(full);
      sem_wait(mutex);
      int item = s->buf[s->out];
      s->out = (s->out + 1) % s->capacity;
      s->count--;
      int left = s->count;
      sem_post(mutex);
      sem_post(empty);

      sem_wait(print);
      printf("Consumer: removed %d (buffer=%d/%d)\n", item, left, s->capacity);
      sem_post(print);
      pause(3);
    }
    exit(0);
  }

  // Producer.
  for (int item = 1; item <= ITEMS; item++) {
    sem_wait(mutex);
    int is_full = (s->count == s->capacity);
    sem_post(mutex);
    if (is_full)
      print_wait(print, "Producer", "buffer full");

    sem_wait(empty);
    sem_wait(mutex);
    s->buf[s->in] = item;
    s->in = (s->in + 1) % s->capacity;
    s->count++;
    int used = s->count;
    sem_post(mutex);
    sem_post(full);

    sem_wait(print);
    printf("Producer: inserted %d (buffer=%d/%d)\n", item, used, s->capacity);
    sem_post(print);
    pause(1);
  }

  wait(0);
  printf("Producer-consumer test complete: %d items processed\n", ITEMS);
  exit(0);
}
