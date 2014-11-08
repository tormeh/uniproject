#include <stdio.h>
#include <stdbool.h>

struct Datastruct
{
  int counter;
};

static void printfoo(struct Datastruct *ds)
{
  while(true)
  {
    printf("foo %d\n", ds->counter);
    ds->counter++;
    return;
  }
}

static void printbaz(struct Datastruct *ds)
{
  while(true)
  {
    printf("baz %d\n", ds->counter);
    ds->counter++;
    return;
  }
}

static void printlol(struct Datastruct *ds)
{
  while(true)
  {
    printf("lol %d\n", ds->counter);
    ds->counter++;
    return;
  }
}

__attribute__ ((noreturn)) static void scheduler(void (*functionPointerArray[])(struct Datastruct *ds), struct Datastruct *ds, int arraylen)
{
  while(true)
  {
    for(int i=0; i<arraylen; i++)
    {
      functionPointerArray[i](&ds[i]);
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
  
  struct Datastruct ds[3];
  ds[0].counter = 0;
  ds[1].counter = 0;
  ds[2].counter = 0;
    
  scheduler(functionPointerArray, &ds[0], 3);
    
  //return 0;
}
