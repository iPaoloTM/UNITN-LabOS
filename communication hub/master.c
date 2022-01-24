
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>

#define RED "\033[0;31m"
#define GREEN "\033[32m"
#define DF "\033[0m"

#define MAXSLAVE 10
#define MAXSTRING 80

int main(int argc, char *argv[]) {

  //rinomina del file compilato da a.out a slave -- serve per execv
  system("gcc slave.c -o slave");

  fprintf(stdout, "master: %d\n", getpid());

  int number;
  int fd;
  char input[MAXSTRING];
  int pid[MAXSLAVE];
  char *masterslave = "fifo1";

  int n=0;
  int m=0;
  char op;
  char str[MAXSTRING];


  unlink(masterslave);

  if (mkfifo(masterslave, S_IRUSR|S_IWUSR) == -1) {perror("Pipe Error:");}//fprintf(stderr, "%sError with fifo1%s\n", RED, DF);}
  //  if (mkfifo(slaveslave, S_IRUSR|S_IWUSR) == -1) {fprintf(stderr, "%sError with fifo2%s\n", RED, DF);}

  if (argc != 2) {
    fprintf(stderr, "%sToo many/few arguments. Usage ./a.out n%s\n", RED, DF);
    exit(1);
  }

  number = atoi(argv[1]);
  //fprintf(stdout, "%s---number %d%s\n", RED, number, DF);

  if ( number > MAXSLAVE || number < 0 ){
    fprintf(stderr, "%sBad argument: 0 < n < %d%s\n", RED, MAXSLAVE, DF);
    exit(2);
  }


  for(int i=0;i<number;i++) // loop will run n times (n=5)
  {
    pid[i]=fork();
    if(pid[i] == 0)
    {


      execv("slave",NULL);
    }
  }

  /*for (int k=0; k<number; k++) {
    printf("%s[#master]%schild %d with pid: %d%s\n", GREEN, DF, k, pid[k]);
  }*/



  while (1 && pid[0] != 0) {


    sleep(1);
    fprintf(stdout, "%sNext command:%s", GREEN, DF);
    fgets(input, MAXSTRING, stdin);

    if(isdigit(input[0]) && isdigit(input[2])){
      n = atoi(&input[0]);
      m = atoi(&input[2]);
      op = input[1];
      strcpy(str, input+sizeof(char)*4);

  if( (op == '>' || op == '<') && ( n > 0 && n < number) && (m > 0 && m < number ) )
  {
    //fprintf(stdout, "I read %d %c %d and %s\n",n,op,m,str);
  fd = open(masterslave, O_RDWR); // Open FIFO for read and write
  if (fd == -1) {fprintf(stderr, "%sError opening pipe masterslave\n%s", RED, DF);} //else {fprintf(stdout, "I opened the pipe \n");}
  if (write(fd, input, strlen(input)+1) == -1) {perror("Error Write: ");} //else {fprintf(stdout, "I wrote %s\n", str);} // write and close
   // close(fd);

   if (op == '>') {
  //n>m
  kill(pid[n], SIGUSR1); //SIGUSR1 => sender
  //printf("inviato segnale SIGUSR1 a %d\n", pid[n]);
  sleep(1);
  kill(pid[m], SIGUSR2); //SIGUSR2 => receiver
  //printf("inviato segnale SIGUSR2 a %d\n", pid[m]);

  } else if (op == '<') {
  //n<m
  kill(pid[m], SIGUSR1); //SIGUSR1 => sender
  //printf("inviato segnale SIGUSR1 a %d\n", pid[m]);
  sleep(1);
  kill(pid[n], SIGUSR2); //SIGUSR2 => receiver
  //printf("inviato segnale SIGUSR2 a %d\n", pid[n]);
  }


} else {
  fprintf(stdout, "%sWrong command%s\n", RED, DF);
}



}
}
   return 0;
}
