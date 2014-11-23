#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

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
  pthread_mutex_t *sched_mutex;
  pthread_mutex_t *ws_mutex;
  void (**functionPointerArray)();
  int arraylen;
  int threadid;
  int currentfunc;
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

static void retfin(struct Comstruct *cs)
{
  cs->ws = FINISHED;
  return;
}

static void busywork()
{
  int j = 5; for(int i=0; i<9000; i++){j=j+i;}
  return;
}

static void printfoo(struct Datastruct *ds, struct Comstruct *cs)
{
  switch(ds->continuation)
  {
    case 0:
      busywork();//printf("foo 0 %d\n", ds->counter);
      retsend(ds, cs, 1, 1, ds->counter);
      return;
    case 1:
      busywork();//printf("foo 1 %d\n", ds->counter);
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
      busywork();//printf("baz 0 %d\n", ds->counter);
      retrecv(ds, cs, 1);
      return;
    case 1:
      busywork();//printf("baz got message with %d\n", cs->message);
      //printf("baz 1 %d\n", ds->counter);
      ds->counter++;
      if (ds->counter >= 6000)
      {
        exit(0);
      }
      retloop(ds, cs);
      return;
  }
}

static void printlol(struct Datastruct *ds, struct Comstruct *cs)
{ cs->ws = cs->ws;
  switch(ds->continuation)
  {
    case 0:
      busywork();//printf("lol 0 %d\n", ds->counter);
      if(rand()%2 == 0)
      {
        retcont(ds, cs, 1);
        return;
      }
    case 1:
      busywork();//printf("lol 1 %d\n", ds->counter);
      ds->counter++;
      if (false /*ds->counter > 1000*/)
      {
        retfin(cs);
        exit(0);
      }
      retloop(ds, cs);
      return;
  }
}

__attribute__ ((noreturn)) static void *scheduler(void *arg)
{
  struct ThreadArgument ta = *(struct ThreadArgument*)arg;
  ta.threadid = rand()%20;
  int blocked = 0;
  //printf("thread with random alias %d started\n", ta.threadid);
  while(true)
  {
    for(int i=0; i<ta.arraylen; i++)
    {
      if (blocked > 10)
      {
        sleep(1);
        blocked = 0;
      }
      if (pthread_mutex_trylock(&ta.ws_mutex[i]) != EBUSY)
      {
        blocked--;
        //printf("thread %d has lock on %d\n", ta.threadid, i);
        if(ta.cs[i].ws == READY)
        {
          ta.cs[i].ws = RUNNING;
          ta.currentfunc = i;
          //printf("thread %d runs %d\n", ta.threadid, i);
          ta.functionPointerArray[i](&ta.ds[i], &ta.cs[i], ta);
        }
        else if(ta.cs[i].ws == SENDING)
        {
          int sender = i;
          int recipient = ta.cs[i].compartner;
          if (pthread_mutex_trylock(&ta.ws_mutex[recipient]) != EBUSY)
          {
            blocked--;
            //printf("thread %d has lock on recipient %d\n", ta.threadid, recipient);
            if(ta.cs[recipient].ws == RECEIVING)
            {
              ta.cs[recipient].message = ta.cs[sender].message;
              ta.cs[recipient].ws = READY;
              pthread_mutex_unlock(&ta.ws_mutex[recipient]);
              ta.cs[sender].ws = RUNNING;
              ta.currentfunc = i;
              //printf("thread %d runs sender %d\n", ta.threadid, i);
              ta.functionPointerArray[i](&ta.ds[i], &ta.cs[i], ta);
            }
            else
            {
              pthread_mutex_unlock(&ta.ws_mutex[recipient]);
            }
          }
          else
          {
            //printf("thread %d denied lock on recipient %d\n", ta.threadid, recipient);
            blocked++;
          }
        }
        pthread_mutex_unlock(&ta.ws_mutex[i]);
      }
      else
      {
        //printf("thread %d denied lock on %d\n", ta.threadid, i);
        blocked++;
      }
    }
  }
}

/*__attribute__ ((noreturn))*/ static void coroutines(void (*functionPointerArray[])(struct Datastruct *ds, struct Comstruct *cs), struct Datastruct *ds, struct Comstruct *cs, int arraylen)
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
  numofthreads = 32;
  
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
    rc = pthread_create(&threads[i], NULL, scheduler, (void *) &ta);
    assert(0 == rc);
  }
  
  for (int i=0; i<numofthreads; ++i) 
  {
    // block until thread i completes
    rc = pthread_join(threads[i], NULL);
    printf("Thread number %d is complete\n", i);
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
