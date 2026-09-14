#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define READERS 3
#define WRITERS 2
#define ROUNDS 3

struct rw_shared {
  int data;
  int read_count;
};

static void
reader(struct rw_shared *s, int read_mutex, int write_lock, int turnstile,
       int print, int id)
{
  for (int r = 0; r < ROUNDS; r++) {
    // Fair gate: once a writer arrives, new readers cannot enter ahead of it.
    sem_wait(turnstile);
    sem_post(turnstile);

    sem_wait(read_mutex);
    s->read_count++;
    int active = s->read_count;
    if (s->read_count == 1)
      sem_wait(write_lock);
    sem_post(read_mutex);

    sem_wait(print);
    printf("Reader %d (pid %d): reading data=%d, active readers=%d\n",
           id, getpid(), s->data, active);
    sem_post(print);
    pause(2);

    sem_wait(read_mutex);
    s->read_count--;
    if (s->read_count == 0)
      sem_post(write_lock);
    sem_post(read_mutex);
    pause(1);
  }
}

static void
writer(struct rw_shared *s, int write_lock, int turnstile, int print, int id)
{
  for (int r = 0; r < ROUNDS; r++) {
    sem_wait(turnstile);
    sem_wait(write_lock);

    s->data = id * 100 + r;
    sem_wait(print);
    printf("Writer %d (pid %d): wrote data=%d, exclusive access\n",
           id, getpid(), s->data);
    sem_post(print);
    pause(3);

    sem_post(write_lock);
    sem_post(turnstile);
    pause(1);
  }
}

int
main(void)
{
  struct rw_shared *s = (struct rw_shared *)shm_get();
  if (s == (void *)-1) {
    printf("shm_get failed\n");
    exit(1);
  }
  s->data = 0;
  s->read_count = 0;

  int read_mutex = sem_create(1);
  int write_lock = sem_create(1);
  int turnstile = sem_create(1);
  int print = sem_create(1);
  if (read_mutex < 0 || write_lock < 0 || turnstile < 0 || print < 0) {
    printf("semaphore creation failed\n");
    exit(1);
  }

  for (int i = 0; i < READERS + WRITERS; i++) {
    int pid = fork();
    if (pid < 0) {
      printf("fork failed\n");
      exit(1);
    }
    if (pid == 0) {
      if (i < READERS)
        reader(s, read_mutex, write_lock, turnstile, print, i + 1);
      else
        writer(s, write_lock, turnstile, print, i - READERS + 1);
      exit(0);
    }
  }

  for (int i = 0; i < READERS + WRITERS; i++)
    wait(0);

  printf("Readers-writers test complete\n");
  exit(0);
}
