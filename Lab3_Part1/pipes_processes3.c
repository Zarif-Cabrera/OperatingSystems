#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(int argc, char **argv){
  if (argc != 2) {
    fprintf(stderr,"Usage %s <grep_argument>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int pipe1[2];
  int pipe2[2];
  pid_t pid1,pid2,pid3;

  pipe(pipe1);
  pipe(pipe2);

  pid1 = fork();

  if (pid1==0){
    dup2(pipe1[1],STDOUT_FILENO);
    close(pipe1[0]);
    close(pipe1[1]);

    char *cat_args[] = {"cat","scores",NULL};
    execvp("cat",cat_args);
  }

  pid2 = fork();

  if (pid2==0){
    dup2(pipe1[0],STDIN_FILENO);
    dup2(pipe2[1],STDOUT_FILENO);
    close(pipe1[1]);
    close(pipe2[0]);

    char *grep_args[] = {"grep",argv[1],NULL};
    execvp("grep",grep_args);
  }

  pid3 = fork();

  if (pid3==0){
    dup2(pipe2[0],STDIN_FILENO);
    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[1]);

    char *sort_args[] = {"sort",NULL};
    execvp("sort",sort_args);
  }

  close(pipe1[0]);
  close(pipe1[1]);
  close(pipe2[0]);
  close(pipe2[1]);

  wait(NULL);
  wait(NULL);
  wait(NULL);

  return 0;
  }

