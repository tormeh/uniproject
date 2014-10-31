#include <stdio.h>
#include <setjmp.h>

static jmp_buf environment1;
static jmp_buf environment2;

__attribute__ ((noreturn)) static void co2()
{
  int VAR = 5;
  printf("VAR = %d\n", VAR);
  
  printf("In function co2 place 1\n");
  
  int ret = setjmp(environment2);
  if (ret == 0)
  {
    longjmp(environment1, 1);
  }
  printf("In function co2 place 2\n");
  ret = setjmp(environment2);
  if (ret == 0)
  {
    longjmp(environment1, 1);
  }
  printf("In function co2 place 3\n");
  printf("VAR = %d\n", VAR);
  longjmp(environment1, 1);
}

static void co1()
{
  printf("In function co1 place 1\n");
  
  int ret = setjmp(environment1);
  if (ret == 0)
  {
    co2();
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
