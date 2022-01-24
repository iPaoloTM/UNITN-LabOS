#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>

#define RED "\033[0;31m"
#define GREEN "\033[32m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define DF "\033[0m"

#define MAXSENDER 10

int pos=0;

typedef struct sender{
  int pid;
  int nUSR1;
  int nUSR2;
} sender;

sender sender_pid[MAXSENDER]; //gloabl pids array for senders

int indexOf(int ps) {
  int index=-1;
  for (int j=0; j < pos; j++) {
    if (sender_pid[j].pid == ps) {
      index = j;
      //fprintf(stdout, "%sfound at index: %d%s\n", MAGENTA, index, DF);
      break;
    }
  }

  return index;
}

void Handler(int signo, siginfo_t *si, void *data) {

  int spid = (int)si->si_pid;

  if (signo == SIGUSR1) {

    fprintf(stdout,"received a SIGUSR1 from %d\n", spid);

    if ((indexOf(spid) == -1) && (pos < MAXSENDER)) {
      sender_pid[pos].pid = spid;
      sender_pid[pos].nUSR1++;
      fprintf(stdout,"%snew sender at index:%d%s\n", MAGENTA, pos, DF);
      pos++;
    } else if ((indexOf(spid) != -1) && (pos < MAXSENDER)) {
      sender_pid[indexOf(spid)].pid = spid;
      sender_pid[indexOf(spid)].nUSR1++;
      fprintf(stdout,"%sold sender at index:%d%s\n", MAGENTA, indexOf(spid), DF);
    } else {
      fprintf(stderr, "Too many sender has already sent signals\n");
    }

  } else if (signo == SIGUSR2) {

    fprintf(stdout,"received a SIGUSR2 from %d\n", (int)si->si_pid);

    if ((indexOf(spid) == -1) && (pos < MAXSENDER)) {
      sender_pid[pos].pid = spid;
      sender_pid[pos].nUSR2++;
      fprintf(stdout,"%snew sender at index:%d%s\n", MAGENTA, pos, DF);
      pos++;
    } else if ((indexOf(spid) != -1) && (pos < MAXSENDER)) {
      sender_pid[indexOf(spid)].pid = spid;
      sender_pid[indexOf(spid)].nUSR2++;
      fprintf(stdout,"%sold sender at index:%d%s\n", MAGENTA, indexOf(spid), DF);
    } else {
      fprintf(stderr, "Too many sender has already sent signals\n");
    }

  } else if (signo == SIGINT || signo == SIGTERM) {

    for (int i = 0; i < pos; i++) {
    fprintf(stdout, "%sProcess with PID %d received %d SIGUSR1 and %d SIGUSR2%s\n", GREEN, sender_pid[i].pid, sender_pid[i].nUSR1, sender_pid[i].nUSR2, DF );
    }
      fprintf(stdout, "Terminating...\n");
      kill(getpid(), SIGKILL);
  }

}

int main(int argc, char const *argv[]) {

  for (int i=0; i < MAXSENDER; i++) {
    sender_pid[i].pid=0;
    sender_pid[i].nUSR1=0;
    sender_pid[i].nUSR2=0;
  }

  struct sigaction sa;

  memset(&sa, 0, sizeof(sa));

  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = Handler;

  //int pid=getpid();

  //fprintf(stdout, "[PID:%d]\n", pid);

  if (sigaction(SIGUSR1, &sa, 0) == -1) {
   fprintf(stderr, "%s: %s\n", "sigaction", strerror(errno));
 }

 if (sigaction(SIGUSR2, &sa, 0) == -1) {
   fprintf(stderr, "%s: %s\n", "sigaction", strerror(errno));
 }

 if (sigaction(SIGINT, &sa, 0) == -1) {
   fprintf(stderr, "%s: %s\n", "sigaction", strerror(errno));
 }

 if (sigaction(SIGTERM, &sa, 0) == -1) {
   fprintf(stderr, "%s: %s\n", "sigaction", strerror(errno));
 }

 printf("Process %d waiting for signals..\n", getpid());


  while(1);

}
