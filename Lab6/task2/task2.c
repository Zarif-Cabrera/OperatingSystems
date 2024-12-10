#include <stdio.h>
#include <stdlib.h>

/* IMPLEMENT ME: Declare your functions here */
int add (int a, int b);
int subtract (int a, int b);
int multiply (int a, int b);
int divide (int a, int b);
void exit_program(int a, int b);


int main (void)
{
  int a = 10, b = 5;
  char operation;

  int (*operations[5])(int,int) = {add,subtract,multiply,divide, (int (*)(int,int)) exit_program};

  printf("Operand 'a' : %d | Operand 'b' : %d\n", a, b);

  while (1) {
    printf("Specify the operation to perform ('0' - add, '1' - subtract, '2' - multiply, '3' - divide, '4' - exit): ");
    scanf(" %c", &operation);

    int op_index = operation - '0';

    operations[op_index](a,b);
  }

  printf("Exiting program.\n");
	return 0;
}

/* IMPLEMENT ME: Define your functions here */
int add (int a, int b) { 
  printf ("Adding 'a' and 'b'\n"); 
  int result = a + b; 
  printf("x = %d\n", result);
  return result;
  }

int subtract (int a, int b) { 
  printf ("Subtracting 'a' and 'b'\n"); 
  int result = a - b; 
  printf("x = %d\n", result);
  return result;
  }

int multiply (int a, int b) { 
  printf ("Multiply 'a' and 'b'\n"); 
  int result = a * b; 
  printf("x = %d\n", result);
  return result;
  }

int divide (int a, int b) { 
  printf ("Dividing 'a' and 'b'\n"); 
  int result = (b != 0) ? a/b : 0; 
  printf("x = %d\n", result);
  return result;
  }

void exit_program(int a, int b) {
  printf("Exiting Program.\n");
  exit(0);
} 
