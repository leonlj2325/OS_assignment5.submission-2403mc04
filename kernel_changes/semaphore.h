#ifndef _SEMAPHORE_H_
#define _SEMAPHORE_H_

#define NSEMS 16

void sem_init_all(void);
int sem_create(int value);
int sem_wait_k(int id);
int sem_post_k(int id);

#endif
