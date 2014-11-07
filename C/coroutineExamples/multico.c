#include <stdio.h>
#include <stdbool.h>

static void printfoo()
{
  while(true)
  {
    printf("foo\n");
    return;
  }
}

static void printbaz()
{
  while(true)
  {
    printf("baz\n");
    return;
  }
}

static void printlol()
{
  while(true)
  {
    printf("lol\n");
    return;
  }
}

__attribute__ ((noreturn)) static void scheduler(void (*functionPointerArray[])(), int arraylen)
{
  while(true)
  {
    for(int i=0; i<arraylen; i++)
    {
      functionPointerArray[i]();
    }
  }
}

int main()
{
  printf("Hello World\n");
    
  //const int arraylen = 3;
  void (*functionPointerArray[3])();
  functionPointerArray[0] = printfoo;
  functionPointerArray[1] = printbaz;
  functionPointerArray[2] = printlol;
    
  scheduler(functionPointerArray, 3);
    
  //return 0;
}
