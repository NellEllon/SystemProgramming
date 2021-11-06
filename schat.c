#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

#define MEM_SIZE 2*(4096+4)

static void forceKill()
{
  wait();
  exit(EXIT_SUCCESS);
}

struct shm_st
{
  int written_0;
  char data_0[BUFSIZ];
  int written_1;
  char data_1[BUFSIZ];
};

int main(int argc, char* argv[])
{
  signal(SIGALRM,forceKill);

  int shmID, child;
  void* sh_mem = NULL;
  struct shm_st* sh_area;
  char buffer[BUFSIZ] = "";

  if(argc < 2)
  {
    fprintf(stderr, "Usage: %s <[1, 2]>\n", *argv);
    exit(EXIT_FAILURE);
  }

  shmID = shmget((key_t)21930, MEM_SIZE, 0666|IPC_CREAT);
  if(shmID == -1)
  {
    perror("shmget() failed\n");
    exit(EXIT_FAILURE);
  }

  sh_mem = shmat(shmID, NULL, 0);
  if (sh_mem == (void *) -1)
  {
    fprintf(stderr, "shmat failed\n");
    exit(EXIT_FAILURE);
  }
  sh_area = (struct shm_st *) sh_mem;

  argv++;
  if(strcmp(*argv, "1") == 0)
  {
    child = fork();
    switch(child)
    {
      case -1: perror("Forking failed\n");
              exit(EXIT_FAILURE);
      case 0 : while(strncmp(buffer,"end chat",8))
              {
                if(sh_area->written_0)
                {
                  memset(buffer, 0, BUFSIZ);
                  strcpy(buffer, sh_area->data_0);
                  write(1,"User 2:", 7);
                  write(1, buffer, BUFSIZ);
                  sh_area->written_0=0;
                }
              }
              kill(child, SIGALRM);
              break;
      default : while(strncmp(buffer, "end chat", 8))
              {
                memset(buffer, 0, BUFSIZ);
                int nbytes = read(0, buffer, BUFSIZ);

                if(nbytes>1&&sh_area)
                {
                  strcpy(sh_area->data_1, buffer);
                  sh_area->written_1=1;
                }
              }
              kill(child, SIGKILL);
              wait();
              break;
    }
  }
  else if (strcmp(*argv, "2") == 0)
  {
    child = fork();
    switch(child)
    {
      case -1: perror("Forking failed\n");
              exit(EXIT_FAILURE);
      case 0 : while(strncmp(buffer, "end chat", 8))
              {
                if(sh_area->written_1)
                {
                  memset(buffer, 0, BUFSIZ);
                  strcpy(buffer, sh_area->data_1);
                  write(1,"User 1:", 7);
                  write(1, buffer, BUFSIZ);
                  sh_area->written_1=0;
                }
              }
              kill(child, SIGALRM);
              break;
      default : while(strncmp(buffer, "end chat", 8))
              {
                memset(buffer, 0, BUFSIZ);
                int nbytes = read(0, buffer, BUFSIZ);

                if(nbytes>1&&sh_area)
                {
                  strcpy(sh_area->data_0, buffer);
                  sh_area->written_0=1;
                }
              }
              kill(child, SIGKILL);
              wait();
              break;
    }
  }

  if (shmdt(sh_mem) == -1 || shmctl(shmID, IPC_RMID, 0) == -1)
  {
    fprintf(stderr, "shmdt or shmctl failed\n");
    exit(EXIT_FAILURE);
  }

  return 0;
}
