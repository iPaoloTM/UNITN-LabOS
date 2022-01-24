#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define RED "\033[0;31m"
#define GREEN "\033[32m"
#define DF "\033[0m"

/*
struct msqid_ds {
  struct ipc_perm msg_perm;   // Ownership and permissions
  time_t msg_stime;  // Time of last msgsnd(2)
  time_t msg_rtime;  // Time of last msgrcv(2)
  time_t msg_ctime; //Time of creation or last modification by msgctl
  unsigned long   msg_cbytes; // # of bytes in queue
  msgqnum_t       msg_qnum;   // # of messages in queue
  msglen_t        msg_qbytes; // Maximum # of bytes in queue
  pid_t           msg_lspid;  // PID of last msgsnd(2)
  pid_t           msg_lrpid;  //PID of last msgrcv(2)
};

struct ipc_perm {
key_t __key;    // Key supplied to msgget(2)
uid_t uid;      // Effective UID of owner
gid_t gid;      // Effective GID of owner
uid_t cuid;     //Effective UID of creator
gid_t cgid;     // Effective GID of creator
unsigned short mode;    // Permissions
unsigned short __seq;   // Sequence number  };
*/

struct msg_buffer{
  long mtype; //Tipo del messaggio
  char mtext[100]; //Buffer che contiene il messaggio
} msgpSND, msgpRCV; //msgpSND -> manda il messaggio, msgpRCV -> riceve il messaggio

int main() {
  struct __msqid_ds_new mod;
  msgpSND.mtype = 1; //Tipo di messaggio
  strcpy(msgpSND.mtext, "This is a message from sender"); //Messaggio da inviare
  key_t queue1Key = ftok("/tmp/queue", 1); //Ottengo chiave univoca
  int queueId = msgget(queue1Key, 0777 | IPC_CREAT); //Creo la coda
  msgctl(queueId, IPC_RMID, NULL); //Rimuove la coda, se esiste già
  queueId = msgget(queue1Key, 0777 | IPC_CREAT); //Creo la coda

  msgsnd(queueId, &msgpSND, sizeof(msgpSND.mtext), 0); //Scrive il messsaggio nella coda
  msgsnd(queueId, &msgpSND, sizeof(msgpSND.mtext), 0); //Scrive il messaggio nella coda
  msgsnd(queueId, &msgpSND, sizeof(msgpSND.mtext), 0); //Scrive il messaggio nella coda

  msgctl(queueId, IPC_STAT, &mod); //Recupera informazioni dal kernel sulla coda e le salva in mod
  printf("Msg in queue: \033[32m %ld\n \033[0mCurrent max bytes in queue: \033[32m %ld\n\n\033[0m ", mod.msg_qnum, mod.msg_qbytes); //Stampo il numero di messaggi della coda e il numero di bytes della coda

  mod.msg_qbytes = 200; //Setto la dimensione massima della coda a 200 bytes
  msgctl(queueId,IPC_SET,&mod); //Applica la modifica appena apportata

  printf("Msg in queue: \033[32m %ld \033[0m--> same number\nCurrent max bytes inqueue: \033[32m %ld\n\n\033[0m",mod.msg_qnum, mod.msg_qbytes); //Stampo il numero di messaggi della coda e il numero di bytes della coda

  if(fork() != 0){ //Il padre scrive nella coda
    printf("\033[32m[SND]\033[0m Sending third message with a full queue...\n");
    msgsnd(queueId, &msgpSND, sizeof(msgpSND.mtext),0); //Scrive il messaggio nella coda
    printf("\033[32m[SND]\033[0mmsg sent\n");
    printf("\033[32m[SND]\033[0mSending fourth message again with IPC_NOWAIT\n");
    if(msgsnd(queueId, &msgpSND, sizeof(msgpSND.mtext),IPC_NOWAIT) == -1) { //Scrive il messaggio nella coda ma se manca lo spazio la chiamata alla funzione fallisce (return -1)
      perror("\033[0;31mQueue is full --> Error\033[0m");
    }
  } else {
    //Il figlio legge dalla coda ogni 3 sec
    sleep(3); msgrcv(queueId, &msgpRCV, sizeof(msgpRCV.mtext),1,0); //Leggo messaggio
    printf("\033[32m[Reader]\033[0m Received msg 1 with msg '%s'\n",msgpRCV.mtext);
    sleep(3); msgrcv(queueId, &msgpRCV, sizeof(msgpRCV.mtext),1,0); //Leggo messaggio
    printf("\033[32m[Reader]\033[0m Received msg 2 with msg '%s'\n",msgpRCV.mtext);
    sleep(3); msgrcv(queueId, &msgpRCV, sizeof(msgpRCV.mtext),1,0); //Leggo messaggio
    printf("\033[32m[Reader]\033[0m Received msg 3 with msg '%s'\n",msgpRCV.mtext);
    sleep(3); msgrcv(queueId, &msgpRCV, sizeof(msgpRCV.mtext),1,0); //Leggo messaggio
    printf("\033[32m[Reader]\033[0m Received msg 4 with msg '%s'\n",msgpRCV.mtext);
    //sleep(3); msgrcv(queueId, &msgpRCV, sizeof(msgpRCV.mtext),1,0); //Leggo messaggio
    //printf("\033[32m[Reader]\033[0m Received msg 5 with msg '%s'\n",msgpRCV.mtext);
    sleep(3);
    if (msgrcv(queueId, &msgpRCV, sizeof(msgpRCV.mtext), 1, IPC_NOWAIT) == -1) { //Leggo il messaggio senza aspettare (se la coda è vuota la chiamata alla funzione fallisce ->  return -1)
      perror("\033[0;31mReader error");
    } else {
      printf("\033[32m[Reader]\033[0m Received msg 5 with msg '%s'\n", msgpRCV.mtext);
    }
  }
  while(wait(NULL)>0);

  return 0;
}
