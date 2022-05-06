#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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

   int instances[10]; //maximum number of resources possible

   int pageTable[32];
   int frameTable[256];
};

struct msgbuf {
   int mtype;
   int message;
};

void fillNeeds(int resourceNeeds[], struct shmseg *shmp); //determines how many of each resources the process needs

int main(int argc, char* argv[]) {

   int resourceNeeds[10];

   int shmid;
   struct shmseg *shmp;
   
   int msgid;
   struct msgbuf buf;

   srand(time(0));

   shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0644|IPC_CREAT);
   if(shmid == -1) {
      perror("user shared memory");
      return 1;
   }

   shmp = shmat(shmid, NULL, 0);
   if(shmp == (void *) -1) {
      perror("user shared memory attach");
   }


   if((msgid = msgget((key_t)12345, 0644 | IPC_CREAT)) == -1) {
      perror("user msgget");
      return 1;
   }

   fillNeeds(resourceNeeds, shmp);

   int j;
   for(j = 0; j < 10; j++) { //outputs resources needs
      printf("child 1 needs %i of resource %i\n", resourceNeeds[j], j);
   }

   int page; //variables for the page and frame finding
   int frame;
   int pageEmpty;
   int frameEmpty;
   pageEmpty = 0;
   frameEmpty = 0;

   do { //loops until empty page is found
      page = (rand() % (32000 - 1 + 1)) + 1; //find random page
      page = page / 1000;

      if(shmp->pageTable[page] == -1) { //if page location is empty

         do { //loops until empty frame is found
            frame = (rand() % (256 - 0 + 1)) + 0; //find random frame

            if(shmp->frameTable[frame] == -1) { //if frame location is empty
               shmp->frameTable[frame] = page;
               shmp->pageTable[page] = frame;
               frameEmpty = 1;
            }

         } while(!frameEmpty);

         pageEmpty = 1;
      }

   } while(!pageEmpty);

   printf("frame location in page array %i\n", shmp->pageTable[page]);
   printf("page location in frame array %i\n", shmp->frameTable[frame]);


   if((msgrcv(msgid, &buf, sizeof(buf), 0, 0)) == -1) {
      perror("user msgrcv");
      return 1;
   }

   printf("user msg %i\n", buf.message);

   buf.message = 3;

 
   if((msgsnd(msgid, &buf, sizeof(buf), 0)) == -1) {
      perror("user msgsnd");
   }

   printf("leaving user\n");
   return 0;
}

void fillNeeds(int resourceNeeds[], struct shmseg *shmp) {
   int i;
   srand(time(0));

   for(i = 0; i < 10; i++) {
      resourceNeeds[i] = (rand() % (shmp->instances[i] - 0 + 1)) + 1;
   }

   return;
}
