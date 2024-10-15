#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

volatile sig_atomic_t alarm_count = 0;
time_t start_time;

void alarm_handler(int signum){
  alarm_count ++;
  printf("Hello World!\n");
  printf("Turing was right!\n");
  alarm(5);
}

void int_handler(int signum){
  time_t end_time = time(NULL);
  printf("\nTotal execution time: %ld seconds \n",end_time-start_time);
  printf("Total SIGALRM signals received %d\n",alarm_count);
  exit(0);
}

int main(int argc, char *argv[]){
  signal(SIGALRM, alarm_handler);
  signal(SIGINT, int_handler);

  start_time = time(NULL);
  alarm(5);

  while (1){
    pause();
  }
  return 0;
}