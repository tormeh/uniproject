#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
//#include <semaphore.h>

typedef enum {READY, SENDING, RECEIVING, FINISHED, RUNNING} Waitstate;

//sem_t sem_scheduler;
static pthread_mutex_t sched_mutex;

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

struct ThreadArgument
{
  struct Datastruct *ds;
  struct Comstruct *cs;
  int arraylen;
  void (*functionPointerArray[])(struct Datastruct *ds, struct Comstruct *cs);
};

static void retsend(struct Datastruct *ds, struct Comstruct *cs, int continuation, int receiver, int message)
{
  cs->compartner = receiver;
  cs->message = message;
  ds->continuation = continuation;
  cs->ws = SENDING;
  return;
}

static void retrecv(struct Datastruct *ds, struct Comstruct *cs, int continuation)
{
  cs->message = -1;
  ds->continuation = continuation;
  cs->ws = RECEIVING;
  return;
}

static void ret(struct Datastruct *ds, struct Comstruct *cs)
{
  ds->continuation = 0;
  cs->ws = READY;
  return;
}

static void retcont(struct Datastruct *ds, struct Comstruct *cs, int continuation)
{
  ds->continuation = continuation;
  cs->ws = READY;
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
      ret(ds, cs);
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
      ret(ds, cs);
      return;
  }
}

static void printlol(struct Datastruct *ds, struct Comstruct *cs)
{ cs->ws = cs->ws;
  switch(ds->continuation)
  {
    case 0:
      printf("lol 0 %d\n", ds->counter);
      if(rand()%2 == 0)
      {
        retcont(ds, cs, 1);
        return;
      }
    case 1:
      printf("lol 1 %d\n", ds->counter);
      ds->counter++;
      if (ds->counter > 10)
      {
        exit(0);
      }
      ret(ds, cs);
      return;
  }
}

__attribute__ ((noreturn)) static void scheduler(void (*functionPointerArray[])(struct Datastruct *ds, struct Comstruct *cs), struct Datastruct *ds, struct Comstruct *cs, int arraylen)
{
  
  while(true)
  {
    for(int i=0; i<arraylen; i++)
    { 
      //sem_wait(&sem_scheduler); //semaphore close; only one thread at a time
      pthread_mutex_lock(&sched_mutex);
      if(cs[i].ws == READY)
      {
        cs[i].ws = RUNNING;
        //sem_post(&sem_scheduler); //semaphore open
        pthread_mutex_unlock(&sched_mutex);
        functionPointerArray[i](&ds[i], &cs[i]);
        continue;
      }
      if(cs[i].ws == SENDING)
      {
        int sender = i;
        int recipient = cs[i].compartner;
        if(cs[recipient].ws == RECEIVING)
        {
          cs[recipient].message = cs[sender].message;
          cs[recipient].ws = READY;
          cs[sender].ws = RUNNING;
          pthread_mutex_unlock(&sched_mutex);
          //sem_post(&sem_scheduler); //semaphore open
          functionPointerArray[i](&ds[i], &cs[i]);
          continue;
        }
      }
    }
  }
}

__attribute__ ((noreturn)) static void coroutines(void (*functionPointerArray[])(struct Datastruct *ds, struct Comstruct *cs), struct Datastruct *ds, struct Comstruct *cs, int arraylen)
{
  //sem_init(&sem_scheduler, 0, 1);
  /*int numofcpus = sysconf(_SC_NPROCESSORS_ONLN);
  int numofthreads = numofcpus;
  pthread_t threads[numofthreads];
  struct ThreadArgument ta;
  ta.ds = ds;
  ta.cs = cs;*/
  
  scheduler(functionPointerArray, &ds[0], &cs[0], arraylen);
  
  
  
  /*for (int i=0; i<numofthreads; ++i) 
  {
    // block until thread i completes
    rc = pthread_join(threads[i], NULL);
    printf("Thread %d is complete\n", i);
    assert(0 == rc);
  }*/
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
  
  coroutines(functionPointerArray, &ds[0], &cs[0], arraylen);
    
  //return 0;
}
