#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<pthread.h>
#include<signal.h>
#include<math.h>
#include<fcntl.h>
#include<ctype.h>

#define SIZE 255
#define MAXCHILD 5

char msg[SIZE];
int count=1;
int queueId=2;
FILE *file;

struct msg_buffer{
    int type;
    char text[100];
}msg1, msg2;

void write_file(int sigNum) {

  file = fopen("/tmp/log.txt", "a");
  if(file == NULL) {perror("File error\n");}
  fprintf(file, "Thread scrive: %d\n", sigNum);
  fclose(file);
}

void* myfun(void* parameter){
  write_file(*(int *)parameter);
    int code=*(int*)parameter;
    int j=1;
    printf("count=%d\n", count);
    printf("msg=%s\n", msg);
    for(j=1; j < count; j++){
        msg1.type = j;
        strcpy(msg1.text, msg);
        msgsnd(queueId, &msg1, strlen(msg1.text), 0);
        //printf("inviato msg(%s) alla coda %d \n", msg, queueId);
    }
    count=1;
}

int isAlphabetic(char c){
    int carattere=c;
    if(isalpha(carattere)){
        return 1;
    }
    else{
        return 0;
    }
}
void myHandler(int code){
    pthread_t id;
    void* retval;
    pthread_create(&id,NULL, myfun, (void*)&code );
    pthread_join(id, &retval);
    printf("La thread ha joinato back\n");
}
 int main(){
     char buffer[SIZE];
     int num;
     int responseSize;
     int isString=0;
     int children[MAXCHILD]={0};
     int i=0;
     int child=0;
     //Creo coda
     remove("/tmp/key.txt");
     creat("/tmp/key.txt", 0777);
     int key=ftok("/tmp/key.txt",1);
     queueId=msgget(key, 0777 | IPC_CREAT );
     while(1){
         i=0;
         child=0;
         sleep(2);
         isString=0;
         printf("[%d] Inserisci stringa o numero\n", getpid());
         memset(buffer,0,SIZE);
         read(0, buffer, SIZE);
         if(strlen(buffer)==1){
             fprintf(stderr, "Riprova, devi inserire una stringa o un numero validi\n");
         }
         else{
             while(!isString && i<SIZE){
                 if(isAlphabetic(buffer[i])){
                     isString=1;
                 }
                 i++;
             }
             if(!isString){
                 num=atoi(buffer);
                 child=fork();
                 if(child==0){

                    printf("Child %d created with count=%d\n", getpid(), count);
                    msgrcv(queueId, &msg2, sizeof(msg2.text),count, 0);
                    perror("Message receive");
                    printf("\t[%d] Ho ricevuto il messaggio %s\n",getpid(), msg2.text);
                    exit(0);
                 }
                 else{
                     signal(SIGUSR1, myHandler);
                     signal(SIGUSR2, myHandler);
                     count++;
                 }
             }
             else{
                 strcpy(msg, buffer);
                 printf("Stringa aggiornata a %s\n",msg );
             }
         }
     }
     msgctl(queueId, IPC_RMID, NULL);
 }
