/* hello_signal.c */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t flag = 0;

void handler(int signum)
{ //signal handler
  printf("Hello World!\n");
  flag = 1;
}

int main(int argc, char * argv[])
{
  signal(SIGALRM,handler); //register handler to handle SIGALRM
  while (1) {
    alarm(5); //Schedule a SIGALRM for 5 seconds
    pause();

    if (flag){
      printf("Turing was right!\n");
      flag = 0;
    }
  }
  return 0; //never reached
}