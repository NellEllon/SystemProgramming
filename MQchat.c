#include "unistd.h"
#include "stdlib.h"
#include "string.h"
#include "fcntl.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "stdio.h"
#include "signal.h"
#include "sys/ipc.h"
#include "sys/msg.h"

#define MAX_BUF 80

typedef struct a_msg
{
  long int msg_type;
  char data[MAX_BUF];
}a_msg;

static void forceKill()
{
  wait();
  exit(EXIT_SUCCESS);
}

int main(int argc,char*argv[])
{
  a_msg a_msg;
  int msg_id, child, nbytes;

  if(argc!=2)
  {
    fprintf(stderr,"Usage: %s <[ 1 2 ]>\n",*argv);
    exit(EXIT_FAILURE);
  }

  msg_id = msgget((key_t)6213146, 0666 | IPC_CREAT);
  if(msg_id==-1)
  {
    fprintf(stderr,"msgget failed");
    exit(EXIT_FAILURE);
  }

  argv++;
  signal(SIGALRM,forceKill);

  if(strcmp(*argv,"1")==0)
  {
    child= fork();
    a_msg.msg_type = 1;
    switch(child)
    {
      case -1: perror("Forking Failed");
               exit(EXIT_FAILURE);
      case 0 : while(strncmp(a_msg.data,"end chat",8))
              {
                if(msgrcv(msg_id, (void *)&a_msg, sizeof(a_msg), 2, 0) == -1)
                {
                  fprintf(stderr,"msgrcv failed");
                  kill(child,SIGALRM);
                  exit(EXIT_FAILURE);
                }
                write(1,"User 2:", 7);
                write(1, a_msg.data, MAX_BUF);
              }
              kill(child, SIGALRM);
              break;
      default : while(strncmp(a_msg.data,"end chat",8))
              {
                memset(a_msg.data, 0, MAX_BUF);
                nbytes = read(0,a_msg.data,MAX_BUF);
                if (nbytes>1)
                {
                  if(msgsnd(msg_id, (void*)&a_msg, sizeof(a_msg), 0) == -1)
                  {
                    fprintf(stderr,"msgsnd failed");
                    kill(child, SIGKILL);
                    wait();
                    exit(EXIT_FAILURE);
                  }
                }
              }
              kill(child,SIGKILL);
              wait();
    }
  }
  else if(strcmp(*argv,"2")==0)
  {
    child= fork();
    a_msg.msg_type = 2;
    switch(child)
    {
      case -1: perror("Forking Failed");
               exit(EXIT_FAILURE);
      case 0 : while(strncmp(a_msg.data,"end chat",8))
              {
                if(msgrcv(msg_id, (void *)&a_msg, sizeof(a_msg), 1, 0) == -1)
                {
                  fprintf(stderr,"msgrcv failed");
                  kill(child,SIGALRM);
                  exit(EXIT_FAILURE);
                }
                write(1,"User 1:", 7);
                write(1, a_msg.data, MAX_BUF);
              }
              kill(child, SIGALRM);
              break;
      default : while(strncmp(a_msg.data,"end chat",8))
              {
                memset(a_msg.data, 0, MAX_BUF);
                nbytes = read(0,a_msg.data,MAX_BUF);
                if (nbytes>1)
                {
                  if(msgsnd(msg_id, (void*)&a_msg, sizeof(a_msg), 0) == -1)
                  {
                    fprintf(stderr,"msgsnd failed");
                    kill(child, SIGKILL);
                    wait();
                    exit(EXIT_FAILURE);
                  }
                }
              }
              kill(child, SIGKILL);
              wait();
    }
  }

  exit(EXIT_SUCCESS);
}
