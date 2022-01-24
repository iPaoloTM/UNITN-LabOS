#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void sigusr1(int signo, siginfo_t *si, void *data) {
  (void)signo;
  (void)data;
  printf("Signal %d from pid %lu\n", (int)si->si_signo,
         (unsigned long)si->si_pid);
  exit(0);
}

int main(void) {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = sigusr1;
  if (sigaction(SIGINT, &sa, 0) == -1) {
    fprintf(stderr, "%s: %s\n", "sigaction", strerror(errno));
  }
  printf("Pid %lu waiting for SIGUSR1\n", (unsigned long)getpid());
  for (;;) {
    sleep(10);
  }
  return 0;
}
