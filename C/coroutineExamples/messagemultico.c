#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

typedef enum {READY, SENDING, RECEIVING} Waitstate;

struct Datastruct
{
  int counter;
  int continuation;
};

struct Comstruct
{
  int compartner;
  int message;
  Waitstate ws;
};

static void retsend(struct Datastruct *ds, int receiver, int message)
{
  ds->ws = SENDING;
  struct Message m;
  m.receiver = receiver;
  m.message = message;
  return m;
}

static void retrecv(struct Datastruct *ds, int receiver)
{
  ds->ws = RECEIVING;
  struct Message m;
  m.receiver = receiver;
  return m;
}

static void printfoo(struct Datastruct *ds, struct Comstruct *cs)
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

static void printbaz(struct Datastruct *ds, struct Comstruct *cs)
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

static void printlol(struct Datastruct *ds, struct Comstruct *cs)
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

__attribute__ ((noreturn)) static void scheduler(void (*functionPointerArray[])(struct Datastruct *ds, struct Comstruct *cs), struct Datastruct *ds, struct Comstruct *cs, int arraylen)
{
  while(true)
  {
    for(int i=0; i<arraylen; i++)
    { 
      if(cs[i]->ws == READY)
      {
        functionPointerArray[i](&ds[i]);
      }
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
    ds[i].ws = READY;
  }
  
  struct Comstruct cs[3];
  for(int i=0; i<3; i++)
  {
    cs[i].ws = READY;
  }
  
  scheduler(functionPointerArray, &ds[0], &cs[0] 3);
    
  //return 0;
}
