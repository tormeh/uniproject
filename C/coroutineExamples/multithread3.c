#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

typedef enum {READY, SENDING, RECEIVING, FINISHED, RUNNING, SLEEPING} Waitstate;

struct Datastruct
{
  int counter;
};

struct Comstruct
{
  int compartner;
  int message;
};

struct Sysstruct
{
  int continuation;
  int compartner;
  Waitstate ws;
};

struct ThreadArgument
{
  struct Datastruct *ds;
  struct Comstruct *cs;
  struct Sysstruct *syss;
  pthread_mutex_t *sched_mutex;
  pthread_mutex_t *ws_mutex;
  void (**functionPointerArray)();
  int arraylen;
  int threadid;
  int currentfunc;
  int numWorkerThreads;
};

static void retsend(struct Comstruct *cs, struct Sysstruct *syss, int continuation, int receiver, int message)
{
  cs->compartner = receiver;
  cs->message = message;
  syss->continuation = continuation;
  syss->ws = SENDING;
  return;
}

static void retrecv(struct Comstruct *cs, struct Sysstruct *syss, int continuation)
{
  cs->message = -1;
  syss->continuation = continuation;
  syss->ws = RECEIVING;
  return;
}

static void retcont(struct Sysstruct *syss, int continuation)
{
  syss->continuation = continuation;
  syss->ws = READY;
  return;
}

static void retloop(struct Sysstruct *syss)
{
  retcont(syss, 0);
  return;
}

static void retfin(struct Sysstruct *syss)
{
  syss->ws = FINISHED;
  return;
}

static void retsleep(struct Sysstruct *syss)
{
  syss->ws = FINISHED;
  return;
}

static void busywork()
{
  int j = 5; for(int i=0; i<9000; i++){j=j+i;}
  return;
}

static void printfoo(struct Datastruct *ds, struct Comstruct *cs, struct Sysstruct *syss)
{
  switch(syss->continuation)
  {
    case 0:
      //printf("foo 0 %d\n", ds->counter);
      retsend(cs, syss, 1, 1, ds->counter);
      return;
    case 1:
      //printf("foo 1 %d\n", ds->counter);
      ds->counter++;
      retloop(syss);
      return;
  }
}

static void printbaz(struct Datastruct *ds, struct Comstruct *cs, struct Sysstruct *syss)
{
  switch(syss->continuation)
  {
    case 0:
      //printf("baz 0 %d\n", ds->counter);
      retrecv(cs, syss, 1);
      return;
    case 1:
      //printf("baz got message with %d\n", cs->message);
      //printf("baz 1 %d\n", ds->counter);
      ds->counter++;
      if (ds->counter >= 6000)
      {
        exit(0);
      }
      retloop(syss);
      return;
  }
}

static void printlol(struct Datastruct *ds, struct Comstruct *cs, struct Sysstruct *syss)
{ //cs->ws = cs->ws;
  switch(syss->continuation)
  {
    case 0:
      //printf("lol 0 %d\n", ds->counter);
      if(rand()%2 == 0)
      {
        retcont(syss, 1);
        return;
      }
    case 1:
      //printf("lol 1 %d\n", ds->counter);
      ds->counter++;
      if (false /*ds->counter > 1000*/)
      {
        retfin(syss);
        exit(0);
      }
      retloop(syss);
      return;
  }
}

__attribute__ ((noreturn)) static void *scheduler(void *arg)
{
  struct ThreadArgument ta = *(struct ThreadArgument*)arg;
  int blocked = 0;
  //printf("thread %ld started\n", ta.threadid);
  while(true)
  {
    for(int i = 0; i<ta.arraylen; i++)
    {
      if (blocked > (ta.arraylen*3))
      {
        //printf("thread %d sleeps. Blocked too often........................................\n", ta.threadid);
        sleep(1);
        blocked = 0;
      }
      else if (blocked < -10000)
      {
        blocked = -1000;
      }
      if (pthread_mutex_trylock(&ta.ws_mutex[i]) != EBUSY)
      {
        blocked--;
        //printf("thread %d has lock on %d\n", ta.threadid, i);
        if(ta.syss[i].ws == READY)
        {
          ta.syss[i].ws = RUNNING;
          ta.currentfunc = i;
          //printf("thread %d runs %d\n", ta.threadid, i);
          ta.functionPointerArray[i](&ta.ds[i], &ta.cs[i], &ta.syss[i], ta);
        }
        else if(ta.syss[i].ws == SENDING)
        {
          int sender = i;
          int recipient = ta.cs[i].compartner;
          if (pthread_mutex_trylock(&ta.ws_mutex[recipient]) != EBUSY)
          {
            blocked--;
            //printf("thread %d has lock on recipient %d\n", ta.threadid, recipient);
            if(ta.syss[recipient].ws == RECEIVING)
            {
              ta.cs[recipient].message = ta.cs[sender].message;
              ta.syss[recipient].ws = READY;
              pthread_mutex_unlock(&ta.ws_mutex[recipient]);
              ta.syss[sender].ws = RUNNING;
              ta.currentfunc = i;
              //printf("thread %d runs sender %d\n", ta.threadid, i);
              ta.functionPointerArray[i](&ta.ds[i], &ta.cs[i], &ta.syss[i], ta);
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
        //printf("thread %d got lock on %d, but the thread couldn't do anything\n", ta.threadid, i);
      }
      else
      {
        //printf("thread %d denied lock on %d\n", ta.threadid, i);
        blocked++;
      }
    }
  }
}

static void coroutines(void (*functionPointerArray[])(struct Datastruct *ds, struct Comstruct *cs, struct Sysstruct *syss), struct Datastruct *ds, struct Comstruct *cs, struct Sysstruct *syss, int arraylen)
{
  long numofcpus = sysconf(_SC_NPROCESSORS_ONLN);
  printf("you have %ld cpus \n", numofcpus);
  printf("you have %d coroutines \n", arraylen);
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
  struct ThreadArgument *ta = malloc((unsigned long)arraylen*sizeof(struct ThreadArgument));
  
  pthread_mutex_t *sched_mutex = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_t *ws_mutex = malloc((unsigned long)arraylen*sizeof(pthread_mutex_t));
  for(int i=0; i<arraylen; i++)
  {
    pthread_mutex_init(&ws_mutex[i], NULL);
    pthread_mutex_unlock(&ws_mutex[i]);
  }
  pthread_mutex_init(sched_mutex, NULL);
  
  int rc;
  for (int i=0; i<(numofthreads); i++) //start threads
  {
    ta[i].ds = ds;
    ta[i].cs = cs;
    ta[i].syss = syss;
    ta[i].arraylen = arraylen;
    ta[i].functionPointerArray = functionPointerArray;
    ta[i].sched_mutex = sched_mutex;
    ta[i].ws_mutex = ws_mutex;
    ta[i].threadid = i;
    ta[i].numWorkerThreads = numofthreads;
    
    printf("Creating thread %d\n", i);
    rc = pthread_create(&threads[i], NULL, scheduler, (void *) &ta[i]);
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
  ws = RUNNING; printf("RUNNING=%d, ", ws);
  ws = SLEEPING; printf("SLEEPING=%d\n", ws);
  
  int arraylen = 3;
  void (**functionPointerArray)() = calloc((unsigned long)arraylen, sizeof( void(*)() ));
  
  functionPointerArray[0] = printfoo;
  functionPointerArray[1] = printbaz;
  functionPointerArray[2] = printlol;
  
  struct Datastruct *ds = malloc((unsigned long)arraylen*sizeof(struct Datastruct));
  struct Comstruct *cs = malloc((unsigned long)arraylen*sizeof(struct Comstruct));
  struct Sysstruct *syss = malloc((unsigned long)arraylen*sizeof(struct Sysstruct));
  for(int i=0; i<arraylen; i++)
  {
    ds[i].counter = 0;
    syss[i].continuation = 0;
    syss[i].ws = READY;
  }
  
  coroutines(functionPointerArray, &ds[0], &cs[0], &syss[0], arraylen);
    
  //return 0;
}
