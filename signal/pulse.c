#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void handler1(int sigNum) {
if (sigNum == SIGCONT) {
      printf("child 1 continued\n");
  }
}

int main() {
  int child1 = fork(); //se io faccio int child1 = !fork(); child1 nel padre vale 0, mentre nel figlio vale 1

  if (child1 != 0)
  printf("child 1 created with pid %d\n", child1);
  int child2;

  if (child1 !=0) {
    child2 = fork();
    if (child2 !=0) printf("child 2 created with pid %d\n", child2);
  }

  signal(SIGCONT, handler1);

  if (child1 == 0 || child2 == 0) {
    while (1) {
      if (child1==0 && child2 != 0) {
        printf("Message1\n");
        sleep(1);
      } else if (child2==0 && child1 != 0) {
        printf("Message2\n");
        sleep(1);
      }
    }
  } else {
    sleep(2);
    printf("stopping child1\n");
    kill(child1, SIGSTOP);
    sleep(2);
    kill(child1, SIGCONT);
    sleep(2);
    kill(child1, SIGKILL);
    kill(child2, SIGKILL);
    printf("children killed\n");
  }

return 0;
}
