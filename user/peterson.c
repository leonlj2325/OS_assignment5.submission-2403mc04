#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

struct shared_state {
  volatile int flag[2];
  volatile int turn;
  volatile int counter;
};

static void
barrier(void)
{
  __sync_synchronize();
}

static void
critical_section(struct shared_state *s, int id)
{
  int other = 1 - id;

  s->flag[id] = 1;
  barrier();
  s->turn = other;
  barrier();

  while (s->flag[other] && s->turn == other)
    ;

  barrier();
  s->counter++;
  printf("Process %d in CS, counter = %d\n", id, s->counter);
  barrier();

  s->flag[id] = 0;
  barrier();
}

int
main(void)
{
  struct shared_state *s = (struct shared_state *)shm_get();
  if (s == (void *)-1) {
    printf("shm_get failed\n");
    exit(1);
  }

  s->flag[0] = 0;
  s->flag[1] = 0;
  s->turn = 0;
  s->counter = 0;
  barrier();

  int pid = fork();
  if (pid < 0) {
    printf("fork failed\n");
    exit(1);
  }

  int id = (pid == 0) ? 1 : 0;
  for (int i = 0; i < 10; i++) {
    critical_section(s, id);
    pause(1);
  }

  if (id == 0) {
    wait(0);
    printf("Final shared counter = %d (expected 20)\n", s->counter);
  }

  exit(0);
}
