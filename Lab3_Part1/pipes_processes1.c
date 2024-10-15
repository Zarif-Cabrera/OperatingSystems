// C program to demonstrate use of fork() and pipe() 
#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 
  
int main() 
{ 
    // We use two pipes 
    // First pipe to send input string from parent 
    // Second pipe to send concatenated string from child 
  
    int fd1[2];  // Used to store two ends of first pipe 
    int fd2[2];  // Used to store two ends of second pipe 
  
    char fixed_str[] = "howard.edu"; 
    char fixed_str2[] = "gobison.org";
    char input_str[100]; 
    pid_t p; 
  
    if (pipe(fd1)==-1) 
    { 
        fprintf(stderr, "Pipe Failed" ); 
        return 1; 
    } 
    if (pipe(fd2)==-1) 
    { 
        fprintf(stderr, "Pipe Failed" ); 
        return 1; 
    } 
  
    printf("Enter a string to concatenate:");
    scanf("%s", input_str); 
    p = fork(); 
  
    if (p < 0) 
    { 
        fprintf(stderr, "fork Failed" ); 
        return 1; 
    } 
  
    // Parent process 
    else if (p > 0) 
    { 
  
        close(fd1[0]);  // Close reading end of pipes 
        close(fd2[1]);
  
        // Write input string and close writing end of first 
        // pipe. 
        write(fd1[1], input_str, strlen(input_str)+1); 
        close(fd1[1]);
  
        // Wait for child to send a string 
        wait(NULL); 
        char final_str[200];
        read(fd2[0],final_str,sizeof(final_str));
        printf("Final string: %s\n", final_str);

        close(fd2[0]); // Close writing end of pipes 
    } 
  
    // child process 
    else
    { 
        close(fd1[1]);  // Close writing end of first pipes 
        close(fd2[0]); 
      
        // Read a string using first pipe 
        char concat_str[200]; 
        read(fd1[0], concat_str, sizeof(concat_str)); 
  
        strcat(concat_str,fixed_str);
  
        printf("Concatenated string %s\n", concat_str);

        char second_input[100];
        printf("Enter another string to concatenate: ");
        scanf("%s",second_input);
        // was confused if you wanted the first string to just have gobison.org added or the second input string added as well, so I did both
        // another option is that the gobison.org is added to second input string, in which case replace concat_str with second_input
        strcat(concat_str, second_input);
        strcat(concat_str,fixed_str2);

        write(fd2[1],concat_str,strlen(concat_str)+1);
        // Close both reading ends 
        close(fd1[0]); 
        close(fd2[1]); 

  
        exit(0); 
    } 
} 