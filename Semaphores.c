#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

static int set_semvalue(pid_t);
static void del_semvalue(pid_t);
static int semaphore_p(pid_t);
static int semaphore_v(pid_t);
static int sem_id1, sem_id2;
void f1(pid_t);
void f2(pid_t);

int mode = 1;

void f1(pid_t id)
{
  if(mode)
    if(!semaphore_p(sem_id1))
      exit(EXIT_FAILURE);
  sleep(1);
  if(id)
    f2(id);
  if(mode)
    if(!semaphore_v(sem_id1))
      exit(EXIT_FAILURE);
}

void f2(pid_t id)
{
  if(mode)
    if(!semaphore_p(sem_id2))
      exit(EXIT_FAILURE);
  sleep(1);
  if(!id)
    f1(id);
  if(mode)
    if(!semaphore_v(sem_id2))
      exit(EXIT_FAILURE);
}

union semun
{
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

int main(void)
{
    sem_id1 = semget((key_t)1234, 1, 0666 | IPC_CREAT);
    sem_id2 = semget((key_t)12, 1, 0666 | IPC_CREAT);

    if (!set_semvalue(sem_id1)&&!set_semvalue(sem_id2))
    {
      fprintf(stderr, "Failed to initialize semaphore\n");
      exit(EXIT_FAILURE);
    }

    printf("Enter 0 to unDeadlock mode\n");
    int temp;
    scanf("%d",&temp);
    if(temp==0)
    {
      mode = 0;
    }

    pid_t id = fork();
    if(id)
    {
      f1(id);
    }
    else
    {
      f2(id);
    }

    printf("Done\n");

    if(id)
    {
      wait();
      del_semvalue(sem_id1);
      del_semvalue(sem_id2);
    }
}

static int set_semvalue(pid_t id)
{
    union semun sem_union;
    sem_union.val = 1;
    if (semctl(id, 0, SETVAL, sem_union) == -1)
        return (0);
    return (1);
}

static void del_semvalue(pid_t id)
{
    union semun sem_union;
    if (semctl(id, 0, IPC_RMID, sem_union) == -1)
        fprintf(stderr, "Failed to delete semaphore\n");
}

static int semaphore_p(pid_t id)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1;
    sem_b.sem_flg = SEM_UNDO;
    if (semop(id, &sem_b, 1) == -1)
    {
        fprintf(stderr, "semaphore_p failed\n");
        return (0);
    }
    return (1);
}

static int semaphore_v(pid_t id)
{
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;
    sem_b.sem_flg = SEM_UNDO;
    if (semop(id, &sem_b, 1) == -1)
    {
        fprintf(stderr, "semaphore_v failed\n");
        return (0);
    }
    return (1);
}
