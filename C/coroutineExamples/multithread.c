#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
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
  void (**functionPointerArray)();
  int threadid;
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

static void retcont(struct Datastruct *ds, struct Comstruct *cs, int continuation)
{
  ds->continuation = continuation;
  cs->ws = READY;
  return;
}

static void retloop(struct Datastruct *ds, struct Comstruct *cs)
{
  retcont(ds, cs, 0);
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
      retloop(ds, cs);
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
      retloop(ds, cs);
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
        retloop(ds, cs);
        return; //exit(0);
      }
      retloop(ds, cs);
      return;
  }
}

__attribute__ ((noreturn)) static void *scheduler(void *arg)
{
  struct ThreadArgument ta = *(struct ThreadArgument*)arg;
  while(true)
  {
    for(int i=0; i<ta.arraylen; i++)
    { 
      //semaphore close; only one thread at a time
      printf("thread %d in scheduler for. WS = ", ta.threadid);
      for(int i=0; i<ta.arraylen; i++)
      {
        printf(" %d", ta.cs[i].ws);
      }
      printf("\n");
      pthread_mutex_lock(&sched_mutex);
      if(ta.cs[i].ws == READY)
      {
        ta.cs[i].ws = RUNNING;
        //semaphore open
        pthread_mutex_unlock(&sched_mutex);
        ta.functionPointerArray[i](&ta.ds[i], &ta.cs[i]);
        continue;
      }
      if(ta.cs[i].ws == SENDING)
      {
        int sender = i;
        int recipient = ta.cs[i].compartner;
        if(ta.cs[recipient].ws == RECEIVING)
        {
          ta.cs[recipient].message = ta.cs[sender].message;
          ta.cs[recipient].ws = READY;
          ta.cs[sender].ws = RUNNING;
          pthread_mutex_unlock(&sched_mutex);
          //semaphore open
          ta.functionPointerArray[i](&ta.ds[i], &ta.cs[i]);
          continue;
        }
      }
    }
  }
}

/*__attribute__ ((noreturn))*/ static void coroutines(void (*functionPointerArray[])(struct Datastruct *ds, struct Comstruct *cs), struct Datastruct *ds, struct Comstruct *cs, int arraylen)
{
  long numofcpus = sysconf(_SC_NPROCESSORS_ONLN);
  long numofthreads = numofcpus;
  pthread_t *threads = malloc((unsigned long)numofthreads*sizeof(pthread_t)); //pthread_t threads[numofthreads];
  struct ThreadArgument ta;
  ta.ds = ds;
  ta.cs = cs;
  ta.arraylen = arraylen;
  ta.functionPointerArray = functionPointerArray; //calloc((unsigned long)arraylen, sizeof( void(*)() ));
  printf("finished fpa cloning \n");
  
  int rc;
  for (int i=0; i<(numofthreads); i++) //start threads
  {
    printf("Creating thread %d\n", i);
    ta.threadid = i;
    rc = pthread_create(&threads[i], NULL, scheduler, (void *) &ta);
    assert(0 == rc);
  }
  
  for (int i=0; i<numofthreads; ++i) 
  {
    // block until thread i completes
    rc = pthread_join(threads[i], NULL);
    printf("Thread %d is complete\n", i);
    assert(0 == rc);
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
  
  coroutines(functionPointerArray, &ds[0], &cs[0], arraylen);
    
  //return 0;
}
