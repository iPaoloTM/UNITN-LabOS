#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>

#define RED "\033[0;31m"
#define GREEN "\033[32m"
#define DF "\033[0m"

#define MAXSTRING 80

char *masterslave = "fifo1";
char *slaveslave = "fifo2";

void handler(int signo, siginfo_t *si, void *data) {

  int n,m;

  char op;

  int fd;
  char input[MAXSTRING];
  char msg[MAXSTRING];
  char mex[MAXSTRING];
    if (signo == SIGUSR1) {

      fd = open(masterslave, O_RDWR);
      if (fd == -1) {fprintf(stderr, "%s[#sender]Error opening pipe masterslave%s\n",   RED,  DF);}
      read(fd, &input, MAXSTRING);

      //fprintf(stdout, "input: %s\n", input);

      close(fd);

      op = input[1];

      strcpy(msg, &input[4]);
      //fprintf(stdout, "msg = %s\n", msg);

      if(op == '>') {//sono n e invio a m
        n = atoi(&input[0]);
        m = atoi(&input[2]);

        sprintf(mex, "%d %s", m, msg);

      } else if (op == '<') {//sono m e invio a n

        m = atoi(&input[0]);
        n = atoi(&input[2]);

        sprintf(mex, "%d %s", m, msg);

      } else {
        printf("Non funziona niente\n");
      }
      //fprintf(stdout, "%s[#sender]%sMaster[%d] told me to send %s\n",GREEN, DF, (int)si->si_pid, msg);

      fd = open(slaveslave, O_RDWR);
      if (fd == -1) {fprintf(stderr, "%s[%d]%sError opening pipe slaveslave\n", RED, n, DF);}
      write(fd, &mex, sizeof(msg));

      fprintf(stdout, "%s[%d]%sSending to %d message: %s\n",GREEN,n, DF , m, msg);

      //close(fd);


        //printf("I'm a sender\n");
    } else if (signo == SIGUSR2) {
        //printf("I'm a receiver\n");
        fd = open(slaveslave, O_RDWR);
        if (fd == -1) {fprintf(stderr, "%s[#receiver]Error opening pipe masterslave%s\n", RED, DF);}
        read(fd, &input, sizeof(input));

        m = atoi(&input[0]);

        strcpy(msg, input+sizeof(char)*2);

        fprintf(stdout, "%s[%d]%sReceived %s\n", GREEN, m, DF, msg);

          close(fd);
    }
}

int main() {

    struct sigaction sa;

  memset(&sa, 0, sizeof(sa));

  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = handler;

  if (sigaction(SIGUSR1, &sa, 0) == -1) {
   fprintf(stderr, "%s: %s\n", "sigaction", strerror(errno));
 }

 if (sigaction(SIGUSR2, &sa, 0) == -1) {
   fprintf(stderr, "%s: %s\n", "sigaction", strerror(errno));
 }

    mkfifo(masterslave, S_IRUSR);
    mkfifo(slaveslave,  S_IRUSR|S_IWUSR);



  while (1) {
    sleep(1);
  }
  fflush(stdout);


    return 0;
}
