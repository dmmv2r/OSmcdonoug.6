
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define SHM_KEY 52

struct shmseg {
   unsigned int seconds;
   unsigned int nanos;

   int instances[10];
};

void clearMem(int shmid, struct shmseg *shmp);

int main(int argc, char* argv[]) {

   int shmid;
   struct shmseg *shmp;

   shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0644|IPC_CREAT);
   if(shmid == -1) {
      perror("oss shared memory");
      return 1;
   }

   shmp = shmat(shmid, NULL, 0);
   if(shmp == (void *) -1) {
      perror("oss sharedmemory attach");
   }


   shmp->seconds = 0;
   shmp->nanos = 0;

   //int *j;
   //j = 1;

   pid_t childpid = 0;
   char* args[] = {"./user_proc", NULL};
   //char* args[] = {"./user_proc", j, NULL};


   if(childpid == fork()) {
      //printf("execing\n");
      execv("./user_proc", args);
   }

   //printf("waiting\n");
   wait(NULL);


   printf("oss seconds %i\n", shmp->seconds);
   printf("oss nanos %i\n", shmp->nanos);

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

