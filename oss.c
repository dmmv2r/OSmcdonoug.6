
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <time.h>

#define SHM_KEY 52
#define PERMS 0644

struct shmseg {
   unsigned int seconds;
   unsigned int nanos;

   int instances[10]; //max number of resource

   int pageTable[32];
   int frameTable[256];
};

struct msgbuf {
   int mtype;
   int message;
};

void fillInstances(struct shmseg *shmp); //randomly creates maximum number of each resource
void fillTables(struct shmseg *shmp); //puts -1 into each table location signifying and empty spot
void clearMem(int shmid, struct shmseg *shmp); //cleans shared memory

int main(int argc, char* argv[]) {

   int shmid;
   struct shmseg *shmp;

   struct msgbuf buf;
   int msgid;

   srand(time(0));

   shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0644|IPC_CREAT);
   if(shmid == -1) {
      perror("oss shared memory");
      return 1;
   }

   shmp = shmat(shmid, NULL, 0);
   if(shmp == (void *) -1) {
      perror("oss sharedmemory attach");
   }


   if((msgid =  msgget((key_t)12345, 0644 | IPC_CREAT)) == -1) {
      perror("oss msgget");
      clearMem(shmid, shmp);
      return 1;
   }

   fillTables(shmp);
   fillInstances(shmp);

   buf.mtype = 1;
   buf.message = 7; //test message

   if((msgsnd(msgid, &buf, sizeof(buf), 0)) == -1) { //sends message to any child
      perror("oss msgsnd");
   }

   shmp->seconds = 0;
   shmp->nanos = 0;

   pid_t childpid = 0;
   char* args[] = {"./user_proc", NULL};

   if(childpid == fork()) { //execs a child process
      execv("./user_proc", args);
   }

   wait(NULL); //waits for child to terminate


   if((msgrcv(msgid, &buf, sizeof(buf), 0, 0)) == -1) { //receives a message from child process
      perror("oss msgrcv");
   }

   printf("oss msg %i\n", buf.message);

   if(msgctl(msgid, IPC_RMID, NULL) == -1) { //cleans message queue
      perror("msgctl");
   }

   clearMem(shmid, shmp);

   printf("ending oss\n");

   return 0;
}

void clearMem(int shmid, struct shmseg *shmp) {
   if(shmdt(shmp) == -1) {
      perror("shmdt");
      return;
   }

   if(shmctl(shmid, IPC_RMID, 0) == -1) {
      perror("shmctl");
      return;
   }

   printf("shared memory cleaned\n");

   return;
}

void fillTables(struct shmseg *shmp) {
   int i;
   for(i = 0; i < 32; i++) {
      shmp->pageTable[i] = -1;
   }

   for(i = 0; i < 256; i++) {
      shmp->frameTable[i] = -1;
   }

   return;
}

void fillInstances(struct shmseg *shmp) {
   srand(time(0));
   int i;

   for(i = 0; i < 10; i++) {
      shmp->instances[i] = (rand() % (20 - 1 + 1)) + 1;
   }

   return;
}
