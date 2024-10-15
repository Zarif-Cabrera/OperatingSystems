#include  <stdio.h>
#include  <string.h>
#include  <sys/types.h>

#define   MAX_COUNT  200
#define   BUF_SIZE   100

void  main(void)
{
     pid_t  pid;
     int    i;
     char   buf[BUF_SIZE];
// # everything under is added from class
//       for (i = 0 ; i < 10; i++){
//         pid = fork();

//         if (pid == 0){
//           pprintf("In Child, %d \t %d \n",i,getpid);

//           if (i==0):
//           printf("Do Somethign proc 0\n");

//           else if (i==1):
//           printf("Do Something proc 1\n");

//         else{
//           printf("Everyone else does something");
//         }
//         exit(1);
//         }


        
//       }

//       for (i==0 ; i < 10; i++)
//         wait(NULL)

//       printf("Parent Finished")

// # everything above is added from class
     fork();
     pid = getpid();
     for (i = 1; i <= MAX_COUNT; i++) {
          sprintf(buf, "This line is from pid %d, value = %d\n", pid, i);
          write(1, buf, strlen(buf));
     } 
}