#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>

#define MAXCHILD 5  //Massimo numero di figli
#define MAXSTRING 128
#define READ 0
#define WRITE 1

int fd[MAXCHILD][2];
int child[MAXCHILD];
int count = 0;
FILE *file;
pthread_t t_id;
char input[MAXSTRING];
char mex[MAXSTRING];


typedef enum {false, true} bool;

char args[MAXSTRING];

//struct sigaction act;

void write_file(int sigNum) {

  file = fopen("/tmp/log.txt", "a");
  if(file == NULL) {perror("File error\n");}


  fprintf(file, "Thread scrive: %d\n", sigNum);
  fclose(file);
}

void *my_fun(void *param){

  write_file(*(int *)param);

  //fprintf(file,"I'm a thread\n");
  //fprintf(file, "SigNo: %d\n", *(int *)param);


        //close(fd[WRITE]);
        for(int i = 0; i <= count; i++) {
          fprintf(file, "Mando mex al figlio: %d\n", child[i]);
            //close(fd[i][READ]);
            memset(mex, 0, MAXSTRING);
            sprintf(mex,"To Child %d PID %d: %s", i, child[i], input);
            //printf("%s\n", mex);
            write(fd[i][WRITE], mex, strlen(mex));
            //close(fd[i][WRITE]);
        }
        printf("This thread received %d\n", *(int *)param);
        return (void*)3;
}


void myHandler(int sigNum) {

  if(sigNum == SIGUSR1 || sigNum == SIGUSR2) {


    printf("[%d]Created a thread\n", getpid());
    pthread_create(&t_id, NULL, my_fun, (void *)&sigNum);
    printf("Executed thread with id %ld\n",t_id);
    pthread_cancel(t_id);

  } else if(sigNum == SIGTERM || sigNum == SIGINT) {
    printf("Stopped with SIG %d\n", sigNum);
  }
}

int main(int argc, char **argv) {
  //leggiamo da stdin

  bool isNumber = true;
  for(int i=0; i<MAXCHILD; i++) {
      if (pipe(fd[i])==-1) {perror("Pipe error\n");}
  }

  while (1) {
    printf("[#PID]%d\n", getpid());
    printf("Write something: ");
    fgets(input, MAXSTRING, stdin);
    printf("I received %s\n", input);

    //controlliamo se è int > 0 oppure stringa
    int input1 = atoi(input);

    //verifico se intero o stringa
    if (input1 > 0) {
      //se int > 0, fork()
      printf("about to birth a child\n");
      if (count+1 < MAXCHILD) {
        count++;
        child[count] = fork();
        if(child[count-1] == -1) {perror("Fork error\n");}
      }

      if (child[count] == 0) {
        printf("I'm a child with PID %d\n", getpid());
        char mex1[MAXSTRING];

        //Figlio
        sleep(2);
        close(fd[count][WRITE]);
        read(fd[count][READ], &mex1, MAXSTRING);
        printf("I've read: %s\n", mex1);

        return 0;

      } else {
        sleep(1);
        //Padre
        //set_handler();
        printf("handler");
        signal(SIGUSR1, myHandler);
        signal(SIGUSR2, myHandler);
      }



    } else {
      printf("New mex saved\n");
      //se string, allora la salvo in args[]
        memset(args, 0, MAXSTRING);
        strcpy(args, input);
    }

  }

  return 0;
}
