// #include  <stdio.h>
// #include  <stdlib.h>
// #include  <sys/types.h>
// #include  <sys/ipc.h>
// #include  <sys/shm.h>
// #include <unistd.h>
// #include <sys/wait.h>

// void  ClientProcess(int []);

// int  main(int  argc, char *argv[])
// {
//      int    ShmID;
//      int    *ShmPTR;
//      pid_t  pid;
//      int    status;

//      if (argc != 5) {
//           printf("Use: %s #1 #2 #3 #4\n", argv[0]);
//           exit(1);
//      }

//      ShmID = shmget(IPC_PRIVATE, 4*sizeof(int), IPC_CREAT | 0666);
//      if (ShmID < 0) {
//           printf("*** shmget error (server) ***\n");
//           exit(1);
//      }
//      printf("Server has received a shared memory of four integers...\n");

//      ShmPTR = (int *) shmat(ShmID, NULL, 0);
//      if (*ShmPTR == -1) {
//           printf("*** shmat error (server) ***\n");
//           exit(1);
//      }
//      printf("Server has attached the shared memory...\n");

//      ShmPTR[0] = atoi(argv[1]);
//      ShmPTR[1] = atoi(argv[2]);
//      ShmPTR[2] = atoi(argv[3]);
//      ShmPTR[3] = atoi(argv[4]);
//      printf("Server has filled %d %d %d %d in shared memory...\n",
//             ShmPTR[0], ShmPTR[1], ShmPTR[2], ShmPTR[3]);

//      printf("Server is about to fork a child process...\n");
//      pid = fork();
//      if (pid < 0) {
//           printf("*** fork error (server) ***\n");
//           exit(1);
//      }
//      else if (pid == 0) {
//           ClientProcess(ShmPTR);
//           exit(0);
//      }

//      wait(&status);
//      printf("Server has detected the completion of its child...\n");
//      shmdt((void *) ShmPTR);
//      printf("Server has detached its shared memory...\n");
//      shmctl(ShmID, IPC_RMID, NULL);
//      printf("Server has removed its shared memory...\n");
//      printf("Server exits...\n");
//      exit(0);
// }

// void  ClientProcess(int  SharedMem[])
// {
//      printf("   Client process started\n");
//      printf("   Client found %d %d %d %d in shared memory\n",
//                 SharedMem[0], SharedMem[1], SharedMem[2], SharedMem[3]);
//      printf("   Client is about to exit\n");
// } 

#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#define NUM_ITERATIONS 25

void ClientProcess(int *sharedMem);

int main(int argc, char *argv[]){
     int    ShmID;
     int    *ShmPTR;
     pid_t  pid;
     int    status;

     // only need 2, BankAccount and Turn
     ShmID = shmget(IPC_PRIVATE, 2*sizeof(int), IPC_CREAT | 0666);
     if (ShmID < 0) {
          printf("*** shmget error (server) ***\n");
          exit(1);
     }

     printf("Server has received shared memory for Bank Account and Turn...\n");

     ShmPTR = (int *) shmat(ShmID, NULL, 0);
     if (*ShmPTR == -1) {
          printf("*** shmat error (server) ***\n");
          exit(1);
     }
     printf("Server has attached the shared memory...\n");

     // initialize to zero
     ShmPTR[0] = 0;
     ShmPTR[1] = 0;
     printf("Server initialized BankAccount and Turn to 0...\n");
     
     printf("Server is about to fork a child process...\n");
     pid = fork();
     if (pid < 0) {
          printf("*** fork error (server) ***\n");
          exit(1);
     }

     if (pid == 0) {
          ClientProcess(ShmPTR);
          exit(0);
     }
     int i;
     for (i = 0; i < NUM_ITERATIONS; i++){
          sleep(rand() % 6);

          int account = ShmPTR[0];

          while (ShmPTR[1] != 0) {
          } 

          if (account <= 100) {
               int balance = rand() % 101;
               if (balance % 2 == 0) {
                    ShmPTR[0] += balance;
                    printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, ShmPTR[0]);
               } else {
                    printf("Dear old Dad: Doesn't have any money to give\n");
          }
     } else {
          printf("Dear Old Dad: Thinks Student has enough Cash ($%d)\n", ShmPTR[0]);
     }

     ShmPTR[1] = 1;

     }
     wait(&status);
     printf("Server has detected the completion of its child...\n");
     shmdt((void *) ShmPTR);
     printf("Server has detached its shared memory...\n");
     shmctl(ShmID, IPC_RMID, NULL);
     printf("Server has removed its shared memory...\n");
     printf("Server exits...\n");
     exit(0);

}

void ClientProcess(int *sharedMem) {
     int i;
     for (i = 0; i < NUM_ITERATIONS;i++){
          sleep(rand() % 6);

          int account = sharedMem[0];

          while (sharedMem[1] != 1){
          }

          int balance = rand() % 51;
          printf("Poor Student needs $%d\n", balance);

          if (balance <= account) {
               sharedMem[0] -= balance;
               printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, sharedMem[0]);
          }else {
               printf("Poor Student: Not Enough Cash ($%d)\n", sharedMem[0] );
          }

          sharedMem[1] = 0;
     }
}