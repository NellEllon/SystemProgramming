#include "unistd.h"
#include "stdlib.h"
#include "string.h"
#include "fcntl.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "stdio.h"
#include "signal.h"

#define FIFO_1 "./fifo1to2"
#define FIFO_2 "./fifo2to1"
#define MAX_RBUF 80

int FIFO_FD1;
int FIFO_FD2;

static void forceKill()
{
  wait();
  exit(EXIT_SUCCESS);
}

int main(int argc,char*argv[])
{
  int child;
  int nbytes;
  char rbuf[MAX_RBUF] ="";
  if(argc!=2)
  {
    fprintf(stderr,"Usage: %s <[ 1 2 ]>\n",*argv);
    exit(EXIT_FAILURE);
  }
  if(access(FIFO_1,F_OK)== -1)
  {
    FIFO_FD1 = mkfifo(FIFO_1,0777);
    if(FIFO_FD1)
    {
      fprintf(stderr,"Could not create fifo %s\n",FIFO_1);
      exit(EXIT_FAILURE);
    }
  }

  if(access(FIFO_2,F_OK)==-1)
  {
    FIFO_FD2 = mkfifo(FIFO_2,0777);
    if(FIFO_FD2)
    {
      fprintf(stderr,"Could not create fifo %s\n",FIFO_2);
      exit(EXIT_FAILURE);
    }
  }

  FIFO_FD1 = open(FIFO_1,O_RDWR);
  FIFO_FD2 = open(FIFO_2,O_RDWR);
  argv++;

  if(strcmp(*argv,"1")==0)
  {
    child= fork();
    switch(child)
    {
      case -1: perror("Forking Failed");
               exit(EXIT_FAILURE);
      case 0 : while(strncmp(rbuf,"end chat",8))
              {
                memset(rbuf,0,sizeof(rbuf));
                nbytes = read(FIFO_FD2,rbuf,MAX_RBUF);
                if (nbytes>1)
                {
                  write(1,"User 2:", 7);
                  write(1, rbuf, nbytes);
                }
              }
              kill(getppid(),SIGALRM);
              break;
      default : while(strncmp(rbuf,"end chat",8))
              {
                memset(rbuf,0,sizeof(rbuf));
                signal(SIGALRM,forceKill);
                nbytes = read(0,rbuf,MAX_RBUF);
                if (nbytes>1)
                {
                  write(FIFO_FD1,rbuf, nbytes);
                }
              }
              kill(child,SIGKILL);
              wait();
    }
  }
  else if(strcmp(*argv,"2")==0)
  {
    child= fork();
    switch(child)
    {
      case -1: perror("Forking Failed");
               exit(EXIT_FAILURE);
      case 0 : while(strncmp(rbuf,"end chat",8))
              {
                memset(rbuf,0,sizeof(rbuf));
                nbytes = read(FIFO_FD1,rbuf,MAX_RBUF);
                if (nbytes>1)
                {
                  write(1,"User 1:", 7);
                  write(1,rbuf, nbytes);
                }
              }
              kill(getppid(),SIGALRM);
              break;
      default : while(strncmp(rbuf,"end chat",8))
              {
                memset(rbuf,0,sizeof(rbuf));
                signal(SIGALRM,forceKill);
                nbytes = read(0,rbuf,MAX_RBUF);
                if (nbytes>1)
                {
                  write(FIFO_FD2,rbuf, nbytes);
                }
              }
              kill(child,SIGKILL);
              wait();
    }
  }

  if(FIFO_FD1 != -1 )
    close(FIFO_FD1);
  if(FIFO_FD2 != -1 )
    close(FIFO_FD2);

  unlink(FIFO_1);
  unlink(FIFO_2);

  exit(EXIT_SUCCESS);
}
