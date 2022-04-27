#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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


int main(int argc, char* argv[]) {

   int shmid;
   struct shmseg *shmp;
   
   shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0644|IPC_CREAT);
   if(shmid == -1) {
      perror("user shared memory");
      return 1;
   }

   shmp = shmat(shmid, NULL, 0);
   if(shmp == (void *) -1) {
      perror("user shared memory attach");
   }

   printf("seconds %i\n", shmp->seconds);
   printf("nanos %i\n", shmp->nanos);

   shmp->seconds = 8;
   shmp->nanos = 4;


   printf("leaving user\n");
   return 0;
}
