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

static void retsend(struct Datastruct *ds, struct Comstruct *cs, int continuation, int receiver, int message)
{
  cs->ws = SENDING;
  cs->compartner = receiver;
  cs->message = message;
  ds->continuation = continuation;
  return;
}

static void retrecv(struct Datastruct *ds, struct Comstruct *cs, int continuation)
{
  cs->ws = RECEIVING;
  cs->message = -1;
  ds->continuation = continuation;
  return;
}

static void printfoo(struct Datastruct *ds, struct Comstruct *cs)
{
  switch(ds->continuation)
  {
    case 0:
      printf("foo 0 %d\n", ds->counter);
      retsend(ds, cs, 1, 1, ds->counter);
      return;
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
      retrecv(ds, cs, 1);
      return;
    case 1:
      printf("baz got message with %d\n", cs->message);
      printf("baz 1 %d\n", ds->counter);
      ds->counter++;
      ds->continuation = 0;
      return;
  }
}

static void printlol(struct Datastruct *ds, struct Comstruct *cs)
{ cs->ws = cs->ws;
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
      if (ds->counter > 10)
      {
        exit(0);
      }
      return;
  }
}

__attribute__ ((noreturn)) static void scheduler(void (*functionPointerArray[])(struct Datastruct *ds, struct Comstruct *cs), struct Datastruct *ds, struct Comstruct *cs, int arraylen)
{
  while(true)
  {
    for(int i=0; i<arraylen; i++)
    { 
      if(cs[i].ws == READY)
      {
        functionPointerArray[i](&ds[i], &cs[i]);
      }
      if(cs[i].ws == SENDING)
      {
        int sender = i;
        int recipient = cs[i].compartner;
        if(cs[recipient].ws == RECEIVING)
        {
          cs[recipient].message = cs[sender].message;
          cs[recipient].ws = READY;
          cs[sender].ws = READY;
          functionPointerArray[i](&ds[i], &cs[i]);
        }
      }
    }
  }
}

int main()
{
  printf("Hello World\n");
  srand((uint) time(NULL));
    
  int arraylen = 3;
  void (**functionPointerArray)() = calloc((unsigned long)arraylen, sizeof( void(*)() ));
  functionPointerArray[0] = printfoo;
  functionPointerArray[1] = printbaz;
  functionPointerArray[2] = printlol;
  
  struct Datastruct *ds = malloc((unsigned long)arraylen*sizeof(struct Datastruct));
  struct Comstruct *cs = malloc((unsigned long)arraylen*sizeof(struct Comstruct));;
  for(int i=0; i<arraylen; i++)
  {
    ds[i].counter = 0;
    ds[i].continuation = 0;
    cs[i].ws = READY;
  }
  
  scheduler(functionPointerArray, &ds[0], &cs[0], arraylen);
    
  //return 0;
}
