#include "types.h"
#include "param.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "semaphore.h"

struct semaphore_table {
  struct spinlock lock;
  int count[NSEMS];
  int used[NSEMS];
};

static struct semaphore_table sems;

void
sem_init_all(void)
{
  initlock(&sems.lock, "semaphores");
  for (int i = 0; i < NSEMS; i++) {
    sems.count[i] = 0;
    sems.used[i] = 0;
  }
}

int
sem_create(int value)
{
  if (value < 0)
    return -1;

  acquire(&sems.lock);
  for (int i = 0; i < NSEMS; i++) {
    if (!sems.used[i]) {
      sems.used[i] = 1;
      sems.count[i] = value;
      release(&sems.lock);
      return i;
    }
  }
  release(&sems.lock);
  return -1;
}

int
sem_wait_k(int id)
{
  if (id < 0 || id >= NSEMS)
    return -1;

  acquire(&sems.lock);
  if (!sems.used[id]) {
    release(&sems.lock);
    return -1;
  }

  while (sems.count[id] == 0) {
    sleep_prepare(&sems.count[id]);
    release(&sems.lock);
    sleep();
    acquire(&sems.lock);
    if (!sems.used[id]) {
      release(&sems.lock);
      return -1;
    }
  }

  sems.count[id]--;
  release(&sems.lock);
  return 0;
}

int
sem_post_k(int id)
{
  if (id < 0 || id >= NSEMS)
    return -1;

  acquire(&sems.lock);
  if (!sems.used[id]) {
    release(&sems.lock);
    return -1;
  }

  sems.count[id]++;
  wakeup(&sems.count[id]);
  release(&sems.lock);
  return 0;
}
