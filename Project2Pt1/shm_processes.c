#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>

#define SHM_SIZE sizeof(int);

void ParentProcess(int  *BankAccount, sem_t *mutex);
void MomProcess(int *BankAccount, sem_t *mutex);
void ChildProcess(int  *BankAccount, sem_t *mutex, int studentNumber);


int main(int  argc, char *argv[])
{
     if (argc != 3) {
          printf("Usage: %s <Number of Parents (1 or 2)> <Number of Children>\n",argv[0]);
          exit(1);
     }

     int numParents = atoi(argv[1]);
     int numChildren = atoi(argv[2]);

     if (numParents < 1 || numParents > 2 || numChildren < 1) {
          printf("Invalid Argument. Must have 1-2 parents and at least 1 child\n");
          exit(1);
     }

     int    ShmID;
     int    *BankAccount;
     pid_t  pid;
     sem_t *mutex;

     srand(time(NULL));

     ShmID = shmget(IPC_PRIVATE, 4*sizeof(int), IPC_CREAT | 0666);
     if (ShmID < 0) {
          printf("*** shmget error (server) ***\n");
          exit(1);
     }
     printf("Server has received a shared memory of four integers...\n");

     BankAccount = (int *) shmat(ShmID, NULL, 0);
     if (*BankAccount == -1) {
          printf("*** shmat error (server) ***\n");
          exit(1);
     }
     printf("Server has attached the shared memory...\n");

     *BankAccount = 0;

     mutex = sem_open("bank_mutex",O_CREAT, 0644,1);
     if (mutex == SEM_FAILED){
          perror("sem_open");
          exit(1);
     }


     printf("Server is about to fork processes...\n");
     for (int i = 0; i < numParents; i++){
          pid = fork();
          if (pid < 0) {
               printf("*** fork error (server) ***\n");
               exit(1);
          }
          else if (pid == 0) {
               if (i == 0) {
                    ParentProcess(BankAccount,mutex); //Dad
               } else {
               MomProcess(BankAccount,mutex); //Mom
          }
          exit(0);
     }
     }

     for (int i = 0; i < numChildren; i++){
          pid = fork();
          if (pid < 0) {
               printf("*** fork error (server) ***\n");
               exit(1);
          }
          else if (pid == 0) {
               ChildProcess(BankAccount,mutex,i+1); //child
               exit(0);
     }
     }

     for (int i = 0; i < numParents + numChildren; i++) {
          wait(NULL);

     }

     sem_close(mutex);
     printf("Server has detected the completion of its child...\n");
     shmdt((void *) BankAccount);
     printf("Server has detached its shared memory...\n");
     shmctl(ShmID, IPC_RMID, NULL);
     printf("Server has removed its shared memory...\n");
     printf("Server exits...\n");
     exit(0);
}

void ParentProcess(int  *BankAccount, sem_t *mutex)
{
     while(1){
          sleep(rand() % 6);

          printf("Dear Old Dad: Attempting to Check Balance\n");

          int localBalance;

          sem_wait(mutex);
          localBalance = *BankAccount;
          sem_post(mutex);

          int randomAction = rand();
          if (randomAction % 2 == 0){
               if (localBalance < 100) {
                    int depositAmount = rand() % 101;
                    if (depositAmount % 2 == 0){
                         localBalance += depositAmount;
                         sem_wait(mutex);
                         *BankAccount = localBalance;
                         sem_post(mutex);
                         printf("Dear old Dad: Deposits $%d / Balance = $%d\n", depositAmount,localBalance);
                    } else {
                         printf("Dear old Dad: Doesn't have any money to give\n");
                    }
               }else{
                    printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", localBalance);
               }
          } else{
               printf("Dear Old Dad: Last Checking Balance = $%d\n", localBalance);
          }
     }
}

void MomProcess(int  *BankAccount, sem_t *mutex)
{
     while(1){
          sleep(rand() % 11);

          printf("Lovable Mom: Attempting to Check Balance\n");

          int localBalance;

          sem_wait(mutex);
          localBalance = *BankAccount;
          sem_post(mutex);

          if (localBalance <= 100) {
               int depositAmount = rand() % 126;
                    localBalance += depositAmount;
                    sem_wait(mutex);
                    *BankAccount = localBalance;
                    sem_post(mutex);
                    printf("Lovable Mom: Deposits $%d / Balance = $%d\n", depositAmount, localBalance);
                    } else {
                    printf("Lovable Mom: Thinks the family has enough cash ($%d)\n", localBalance);
                    }
     }
}


void ChildProcess(int  *BankAccount, sem_t *mutex, int studentNumber)
{
 while(1){
          sleep(rand() % 6);

          printf("Poor Student #%d: Attempting to Check Balance\n", studentNumber);

          int localBalance;

          sem_wait(mutex);
          localBalance = *BankAccount;
          sem_post(mutex);

          int randomAction = rand();
          if (randomAction % 2 == 0){
               int need = rand() % 51;
               printf("Poor Student #%d needs $%d\n",studentNumber, need);

               if (need <= localBalance) {
                    localBalance -= need;
                    sem_wait(mutex);
                    *BankAccount = localBalance;
                    sem_post(mutex);
                    printf("Poor Student #%d: Withdraws $%d / Balance = $%d\n", studentNumber, need, localBalance);
                    } else {
                         printf("Poor Student #%d: Not Enough Cash ($%d)\n", studentNumber, localBalance);
                    }
               } else {
                    printf("Poor Student #%d: Last Checking Balance = $%d\n", studentNumber, localBalance);
               }
          }
}

//for Checking

// void ParentProcess(int *BankAccount, sem_t *mutex) {
//     while (1) {
//         sleep(rand() % 6);

//         printf("Dear Old Dad: Attempting to Check Balance\n");

//         sem_wait(mutex);
//         printf("Dear Old Dad: Entering critical section\n");
//         int localBalance = *BankAccount;
//         printf("Dear Old Dad: Inside critical section. Current Balance = $%d\n", localBalance);
//         sem_post(mutex);
//         printf("Dear Old Dad: Exiting critical section\n");

//         int randomAction = rand();
//         if (randomAction % 2 == 0) {
//             if (localBalance < 100) {
//                 int depositAmount = rand() % 101;
//                 if (depositAmount % 2 == 0) {
//                     localBalance += depositAmount;

//                     sem_wait(mutex);
//                     printf("Dear Old Dad: Entering critical section to update balance\n");
//                     *BankAccount = localBalance;
//                     printf("Dear Old Dad: Updated Balance = $%d\n", localBalance);
//                     sem_post(mutex);
//                     printf("Dear Old Dad: Exiting critical section after update\n");

//                     printf("Dear old Dad: Deposits $%d / Balance = $%d\n", depositAmount, localBalance);
//                 } else {
//                     printf("Dear old Dad: Doesn't have any money to give\n");
//                 }
//             } else {
//                 printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", localBalance);
//             }
//         } else {
//             printf("Dear Old Dad: Last Checking Balance = $%d\n", localBalance);
//         }
//     }
// }


// void ChildProcess(int *BankAccount, sem_t *mutex) {
//     while (1) {
//         sleep(rand() % 6);

//         printf("Poor Student: Attempting to Check Balance\n");

//         sem_wait(mutex);
//         printf("Poor Student: Entering critical section\n");
//         int localBalance = *BankAccount;
//         printf("Poor Student: Inside critical section. Current Balance = $%d\n", localBalance);
//         sem_post(mutex);
//         printf("Poor Student: Exiting critical section\n");

//         int randomAction = rand();
//         if (randomAction % 2 == 0) {
//             int need = rand() % 51;
//             printf("Poor Student needs $%d\n", need);

//             if (need <= localBalance) {
//                 localBalance -= need;

//                 sem_wait(mutex);
//                 printf("Poor Student: Entering critical section to update balance\n");
//                 *BankAccount = localBalance;
//                 printf("Poor Student: Updated Balance = $%d\n", localBalance);
//                 sem_post(mutex);
//                 printf("Poor Student: Exiting critical section after update\n");

//                 printf("Poor Student: Withdraws $%d / Balance = $%d\n", need, localBalance);
//             } else {
//                 printf("Poor Student: Not Enough Cash ($%d)\n", localBalance);
//             }
//         } else {
//             printf("Poor Student: Last Checking Balance = $%d\n", localBalance);
//         }
//     }
// }
