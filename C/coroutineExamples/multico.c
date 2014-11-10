#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

struct Datastruct
{
  int counter;
  int continuation;
};

static void printfoo(struct Datastruct *ds)
{
  switch(ds->continuation)
  {
    case 0:
      printf("foo 0 %d\n", ds->counter);
      ds->continuation = 1;
      if(rand()%2 == 0)
      {
        return;
      }
    case 1:
      printf("foo 1 %d\n", ds->counter);
      ds->counter++;
      ds->continuation = 0;
      return;
  }
}

static void printbaz(struct Datastruct *ds)
{
  switch(ds->continuation)
  {
    case 0:
      printf("baz 0 %d\n", ds->counter);
      ds->continuation = 1;
      if(rand()%2 == 0)
      {
        return;
      }
    case 1:
      printf("baz 1 %d\n", ds->counter);
      ds->counter++;
      ds->continuation = 0;
      return;
  }
}

static void printlol(struct Datastruct *ds)
{
  switch(ds->continuation)
  {
    case 0:
      printf("lol 0 %d\n", ds->counter);
      ds->continuation = 1;
      if(rand()%2 == 0)
      {
        return;
      }
    case 1:
      printf("lol 1 %d\n", ds->counter);
      ds->counter++;
      ds->continuation = 0;
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
  srand((uint) time(NULL));
    
  //const int arraylen = 3;
  void (*functionPointerArray[3])();
  functionPointerArray[0] = printfoo;
  functionPointerArray[1] = printbaz;
  functionPointerArray[2] = printlol;
  
  struct Datastruct ds[3];
  for(int i=0; i<3; i++)
  {
    ds[i].counter = 0;
    ds[i].continuation = 0;
  }
    
  scheduler(functionPointerArray, &ds[0], 3);
    
  //return 0;
}
