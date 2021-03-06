#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
//#include <semaphore.h>

typedef enum {READY, SENDING, RECEIVING, FINISHED, RUNNING} Waitstate;

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
  pthread_mutex_t *sched_mutex;
  pthread_mutex_t *ws_mutex;
  int currentfunc;
};

static void retsend(struct Datastruct *ds, struct Comstruct *cs, struct ThreadArgument ta, int continuation, int receiver, int message)
{
  cs->compartner = receiver;
  cs->message = message;
  ds->continuation = continuation;
  pthread_mutex_lock(&ta.ws_mutex[ta.currentfunc]);
  cs->ws = SENDING;
  pthread_mutex_unlock(&ta.ws_mutex[ta.currentfunc]);
  return;
}

static void retrecv(struct Datastruct *ds, struct Comstruct *cs, struct ThreadArgument ta, int continuation)
{
  cs->message = -1;
  ds->continuation = continuation;
  pthread_mutex_lock(&ta.ws_mutex[ta.currentfunc]);
  cs->ws = RECEIVING;
  pthread_mutex_unlock(&ta.ws_mutex[ta.currentfunc]);
  return;
}

static void retcont(struct Datastruct *ds, struct Comstruct *cs, struct ThreadArgument ta, int continuation)
{
  ds->continuation = continuation;
  pthread_mutex_lock(&ta.ws_mutex[ta.currentfunc]);
  cs->ws = READY;
  pthread_mutex_unlock(&ta.ws_mutex[ta.currentfunc]);
  return;
}

static void retloop(struct Datastruct *ds, struct Comstruct *cs, struct ThreadArgument ta)
{
  retcont(ds, cs, ta, 0);
  return;
}

static void retfin(struct Comstruct *cs, struct ThreadArgument ta)
{
  pthread_mutex_lock(&ta.ws_mutex[ta.currentfunc]);
  cs->ws = FINISHED;
  pthread_mutex_unlock(&ta.ws_mutex[ta.currentfunc]);
  return;
}

static void printfoo(struct Datastruct *ds, struct Comstruct *cs, struct ThreadArgument ta)
{
  switch(ds->continuation)
  {
    case 0:
      printf("foo 0 %d\n", ds->counter);
      retsend(ds, cs, ta, 1, 1, ds->counter);
      return;
    case 1:
      printf("foo 1 %d\n", ds->counter);
      ds->counter++;
      retloop(ds, cs, ta);
      return;
  }
}

static void printbaz(struct Datastruct *ds, struct Comstruct *cs, struct ThreadArgument ta)
{
  switch(ds->continuation)
  {
    case 0:
      printf("baz 0 %d\n", ds->counter);
      retrecv(ds, cs, ta, 1);
      return;
    case 1:
      printf("baz got message with %d\n", cs->message);
      printf("baz 1 %d\n", ds->counter);
      ds->counter++;
      retloop(ds, cs, ta);
      return;
  }
}

static void printlol(struct Datastruct *ds, struct Comstruct *cs, struct ThreadArgument ta)
{ cs->ws = cs->ws;
  switch(ds->continuation)
  {
    case 0:
      printf("lol 0 %d\n", ds->counter);
      if(rand()%2 == 0)
      {
        retcont(ds, cs, ta, 1);
        return;
      }
    case 1:
      printf("lol 1 %d\n", ds->counter);
      ds->counter++;
      if (ds->counter > 10)
      {
        retfin(cs, ta);
        exit(0);
      }
      retloop(ds, cs, ta);
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
      pthread_mutex_lock((ta.sched_mutex));
      printf("thread %d in scheduler for. WS =", ta.threadid);
      for(int j=0; j<ta.arraylen; j++)
      {
        printf("%d,", ta.cs[j].ws);
      }
      printf("\n");
      if(ta.cs[i].ws == READY)
      {
        ta.cs[i].ws = RUNNING;
        //semaphore open
        pthread_mutex_unlock((ta.sched_mutex));
        ta.currentfunc = i;
        ta.functionPointerArray[i](&ta.ds[i], &ta.cs[i], ta);
        continue;
      }
      if(ta.cs[i].ws == SENDING)
      {
        int sender = i;
        int recipient = ta.cs[i].compartner;
        if(ta.cs[recipient].ws == RECEIVING)
        {
          //pthread_mutex_lock(&ta.ws_mutex[recipient]);
          //pthread_mutex_lock(&ta.ws_mutex[sender]);
          ta.cs[recipient].message = ta.cs[sender].message;
          ta.cs[recipient].ws = READY;
          ta.cs[sender].ws = RUNNING;
          pthread_mutex_unlock((ta.sched_mutex));
          //semaphore open
          ta.currentfunc = i;
          ta.functionPointerArray[i](&ta.ds[i], &ta.cs[i], ta);
          continue;
        }
      }
    }
  }
}

/*__attribute__ ((noreturn))*/ static void coroutines(void (*functionPointerArray[])(struct Datastruct *ds, struct Comstruct *cs, struct ThreadArgument ta), struct Datastruct *ds, struct Comstruct *cs, int arraylen)
{
  long numofcpus = sysconf(_SC_NPROCESSORS_ONLN);
  long numofthreads;
  if (arraylen > numofcpus)
  {
    numofthreads = numofcpus;
  }
  else
  {
    numofthreads = (long)arraylen;
  }
  
  pthread_t *threads = malloc((unsigned long)numofthreads*sizeof(pthread_t)); //pthread_t threads[numofthreads];
  struct ThreadArgument ta;
  ta.ds = ds;
  ta.cs = cs;
  ta.arraylen = arraylen;
  ta.functionPointerArray = functionPointerArray;
  ta.sched_mutex = malloc(sizeof(pthread_mutex_t));
  ta.ws_mutex = malloc((unsigned long)arraylen*sizeof(pthread_mutex_t));
  for(int i=0; i<ta.arraylen; i++)
  {
    pthread_mutex_init(&ta.ws_mutex[i], NULL);
    pthread_mutex_unlock(&ta.ws_mutex[i]);
  }
  pthread_mutex_init(ta.sched_mutex, NULL);
  
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
  srand((unsigned int) time(NULL));
  
  printf("WS legend: ");
  Waitstate ws = READY; printf("READY=%d, ", ws);
  ws = SENDING; printf("SENDING=%d, ", ws);
  ws = RECEIVING; printf("RECEIVING=%d, ", ws);
  ws = FINISHED; printf("FINISHED=%d, ", ws);
  ws = RUNNING; printf("RUNNING=%d\n", ws);
  
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
