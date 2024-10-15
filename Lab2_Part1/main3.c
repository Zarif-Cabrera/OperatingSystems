#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define MAX_ITERATIONS 30
#define MAX_SLEEP 10
// test only passes if you are running ./my3proc for some reason, gotta manually start it
void  ChildProcess(void){
  srandom(time(NULL) ^ (getpid() << 16));

  int iterations = random() % (MAX_ITERATIONS + 1);
  int i;
  for (i = 0; i < iterations; i++) {
    pid_t pid = getpid();
    pid_t ppid = getppid();

    printf("Child Pid: %d is going to sleep\n",pid);

    int sleep_time = random() % (MAX_SLEEP+1);
    sleep(sleep_time);

    printf("Child Pid: %d is awake.\n Where is my Parent: %d.\n",pid,ppid);

  }
  exit(0);
}

int  main(void) {
  pid_t pid1,pid2;

  // Fork first child
  pid1 = fork();
  if (pid1 == 0) {
    ChildProcess();
  }

  // Fork second child
  if (pid2 == 0){
    ChildProcess();
  }

  // Parent waits for children to complete
  int status;
  pid_t completed_pid;
  int i;

  for (i = 0; i < 2;i ++){
    completed_pid = wait(&status);
    printf("Child Pid: %d has completed\n",completed_pid);

  }

  return 0;
}
