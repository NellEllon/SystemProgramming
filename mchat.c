#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stddef.h>

#define FILE_LENGTH 2*(BUFSIZ+9)

static void forceKill()
{
  wait(NULL);
  exit(EXIT_SUCCESS);
}

struct mm_st
{
  int nbytes_0;
  int written_0;
  char data_0[BUFSIZ];
  int nbytes_1;
  int written_1;
  char data_1[BUFSIZ];
};

int main(int argc, char* argv[])
{
  signal(SIGALRM,forceKill);

  int child,fd;
  void* file_mem = NULL;
  char buffer[BUFSIZ] = " ";

  if(argc < 2)
  {
    fprintf(stderr, "Usage: %s <[1, 2]>\n", *argv);
    exit(EXIT_FAILURE);
  }

  fd = open ("Chat log", O_RDWR | O_CREAT , S_IRUSR | S_IWUSR);
  if(fd<0)
  {
    perror("Open file error");
    exit(1);
  }

  lseek (fd, FILE_LENGTH, SEEK_SET);
  write (fd, "", 1);
  lseek (fd, 0, SEEK_SET);

  file_mem = mmap(NULL, FILE_LENGTH, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  if(file_mem==MAP_FAILED)
  {
    perror("mmap() failed\n");
    exit(EXIT_FAILURE);
  }

  struct mm_st* mm_area;
  mm_area = (struct mm_st*)file_mem;
  memset(mm_area, 0, FILE_LENGTH);
  mm_area->written_0=0;
  mm_area->written_1=0;
  mm_area->data_0[0]='\n';
  mm_area->data_1[0]='\n';
  mm_area->nbytes_0=0;
  mm_area->nbytes_1=0;

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
                if(mm_area->written_0)
                {
                  memset(buffer, 0, BUFSIZ);

                  int i = mm_area->nbytes_0;
                  int j;
                  do
                  {
                    i--;
                  }
                  while(mm_area->data_0[i]!='\n');


                  write(1,"User 2:", 7);
                  for(j=0, i=i+1;i<mm_area->nbytes_0+1;i++,j++)
                  {
                    buffer[j] = mm_area->data_0[i];
                  }
                  write(1, buffer, j);
                  mm_area->written_0 = 0;
                }
              }
              kill(child, SIGALRM);
              break;
      default : while(strncmp(buffer, "end chat", 8))
              {
                memset(buffer, 0, BUFSIZ);
                int nbytes = read(0, buffer, BUFSIZ);

                if(nbytes>1)
                {
                  mm_area->nbytes_1+=nbytes;
                  strcat(mm_area->data_1, buffer);
                  mm_area->written_1 = 1;
                }
              }
              kill(child, SIGKILL);
              wait(NULL);
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
      case 0 :
              while(strncmp(buffer,"end chat",8))
              {
                if(mm_area->written_1)
                {
                  memset(buffer, 0, BUFSIZ);

                  int i = mm_area->nbytes_1;
                  int j;
                  do
                  {
                    i--;
                  }
                  while(mm_area->data_1[i]!='\n');

                  write(1,"User 1:", 7);
                  for(j=0, i=i+1;i<mm_area->nbytes_1+1;i++,j++)
                  {
                    buffer[j] = mm_area->data_1[i];
                  }
                  write(1, buffer, j);
                  mm_area->written_1 = 0;
                }
              }
              kill(child, SIGALRM);
              break;
      default : while(strncmp(buffer, "end chat", 8))
              {
                memset(buffer, 0, BUFSIZ);
                int nbytes = read(0, buffer, BUFSIZ);

                if(nbytes>1)
                {
                  mm_area->nbytes_0+=nbytes;
                  strcat(mm_area->data_0, buffer);
                  mm_area->written_0 = 1;
                }
              }
              kill(child, SIGKILL);
              wait(NULL);
              break;
    }
  }

  munmap (file_mem, FILE_LENGTH);
  close(fd);
  return 0;
}
