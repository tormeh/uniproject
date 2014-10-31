#include<stdio.h>
#include <setjmp.h>

jmp_buf environment1;
jmp_buf environment2;

void co2(jmp_buf *venvironment1, jmp_buf *venvironment2)
{
  int VAR = 5; //segfault if this line removed <<<<<<<<<<<<<<<<<<<<<<
  jmp_buf * const environment1 = venvironment1;
  jmp_buf * const environment2 = venvironment2;
  printf("In function co2 place 1\n");
  
  int ret = setjmp(*environment2);
  if (ret == 0)
  {
    longjmp(*environment1, 1);
  }
  printf("In function co2 place 2\n");
  ret = setjmp(*environment2);
  if (ret == 0)
  {
    longjmp(*environment1, 1);
  }
  printf("In function co2 place 3\n");
  longjmp(*environment1, 1);
}

void co1()
{
  printf("In function co1 place 1\n");
  
  int ret = setjmp(environment1);
  if (ret == 0)
  {
    co2(&environment1, &environment2);
  }
  printf("In function co1 place 2\n");
  
  ret = setjmp(environment1);
  if (ret == 0)
  {
    longjmp(environment2, 1);
  }
  printf("In function co1 place 3\n");
  ret = setjmp(environment1);
  if (ret == 0)
  {
    longjmp(environment2, 1);
  }
  return;
}

int main()
{
    printf("Hello World\n");
    co1();
    return 0;
}
