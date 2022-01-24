#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#define RED "\033[0;31m"
#define GREEN "\033[32m"
#define MAGIC_COLOR0 "\033[38m"
#define MAGIC_COLOR1 "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define DF "\033[0m"

#define READ 0
#define WRITE 1
#define MAX_LENGTH 100
#define MAX_FIGLI 20

//pipe bidirezionali
int fd1_pipe[MAX_FIGLI][2]; //padre scrive, figlio legge
int fd2_pipe[MAX_FIGLI][2]; //figlio scrive, padre legge

void sendRandom(int n) {

  srand(time(NULL));
  int num = rand();

  char str[MAX_LENGTH];
  sprintf(str, "%d", num);

  fprintf(stdout, "%sChild computing random....%s\n", GREEN, DF);
  write(fd2_pipe[n][WRITE], str, strlen(str));
}

void sendId(int n) {

  int pid = getpid();

  char str[MAX_LENGTH];
  sprintf(str, "%d", pid);

  fprintf(stdout, "%sChild sending own pid....%s\n", GREEN, DF);
  write(fd2_pipe[n][WRITE], str, strlen(str));
}

void quit(int n, int child_pid[]) {

  for(int i = 0; i < n; i++) {
    kill(child_pid[i], SIGKILL);
  }

}

int main(int argc, char const *argv[]) {

  if(argc != 2) {
    fprintf(stderr, "%sToo many/few arguments, required 1%s\n", RED, DF);
    exit(2);
  }

  //ricevo numero di figli
  int n = atoi(argv[1]);
  if(n < 1 || n > MAX_FIGLI) {
    fprintf(stderr, "%sWrong number specified%s\n", RED, DF);
    exit(3);
  }

  //array per pid figli
  int child_pid[MAX_FIGLI];
  int father = getpid();

  fprintf(stdout, "%s[MAIN] %d%s\n", GREEN, father, DF);
  fprintf(stdout, "%sCreating %s%d %sprocesses%s\n", CYAN, MAGIC_COLOR0, n, MAGIC_COLOR1, DF);

  for(int i = 0; i < n; i++) {
    pipe(fd1_pipe[i]);
    pipe(fd2_pipe[i]);
  }

  for(int i = 0; i < n; i++) {

    child_pid[i] = fork();
    if(child_pid[i] == -1) {fprintf(stderr, "%sFork error%s\n", RED, DF); exit(4);}

    if(child_pid[i] == 0) {
      break;
    }
  }

  for(int i = 0; i < n; i++) {

    if(child_pid[i] == 0) {

      while(1) {

        char buf[MAX_LENGTH];
        memset(buf, 0, MAX_LENGTH);

        close(fd1_pipe[i][WRITE]);
        close(fd2_pipe[i][READ]);

        int r = read(fd1_pipe[i][READ], &buf, MAX_LENGTH);

        int numero = atoi(&buf[1]);

        switch(buf[0]) {
          case 'i': sendId(i); break;
          case 'r': sendRandom(i); break;
          default: fprintf(stderr, "%sBad command%s\n", RED, DF);
        }

      }
    }
  }

  while(1) {

    char command[MAX_LENGTH];
    memset(command, 0, MAX_LENGTH);

    fprintf(stdout, "%sNext command: %s", MAGENTA, DF);
    fscanf(stdin, "%s", command);

    char arg1 = command[0];
    char arg2[MAX_LENGTH];
    memset(arg2, 0, MAX_LENGTH);

    strncpy(arg2, &command[1], strlen(command)-1);

    int res = atoi(arg2);
    if(res < 0 || res >= n) {
      fprintf(stderr, "%sWrong target%s\n", RED, DF);
      continue;
    }

    if(arg1 == 'i' || arg1 == 'r') {

      close(fd1_pipe[res][READ]);
      close(fd2_pipe[res][WRITE]);

      write(fd1_pipe[res][WRITE], command, strlen(command));

      char b[MAX_LENGTH];
      memset(b, 0, MAX_LENGTH);

      int r = read(fd2_pipe[res][READ], &b, sizeof(b));
      fprintf(stdout, "Child %d told me: %s\n", child_pid[res], b);

    } else if(arg1 == 'q') {

      quit(n, child_pid);
      fprintf(stdout, "Terminated\n");
      return 0;

    } else {
      fprintf(stderr, "%sWrong parameter. Allowed are 'r', 'i' and 'q'%s\n", RED, DF);
    }
  }

  return 0;
}
