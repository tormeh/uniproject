#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>


void intmap(int array[], int arrayLength, int (*function)(int) )
{
  for(int i = 0; i < arrayLength; i++)
  {
    array[i] = function(array[i]);
  }
}

struct ThreadArgument
{
  int numIterations;
  int *iterationsStart;
  int (*function)(int);
};

void *singlethreaditerator(void *arg) //struct ThreadArgument* argument
{ 
  printf("Entered singlethreaditerator\n");
  struct ThreadArgument argument = *(struct ThreadArgument*)arg;
  for(int i=0; i<argument.numIterations; i++)
  {
    *(argument.iterationsStart+i) = argument.function(*(argument.iterationsStart+i));
  }
  return NULL;
}

void intparmap(int array[], int arrayLength, int (*function)(int) )
{
  int numofcpus = sysconf(_SC_NPROCESSORS_ONLN);
  int numofthreads = numofcpus;
  pthread_t threads[numofthreads];
  int thread_args[numofthreads];
  int tasksPerThread = arrayLength/numofthreads;
  struct ThreadArgument* threadsarguments[numofthreads];

  for (int i=0; i<(numofthreads-1); i++) //handle first n-1 threads
  {
    printf("intparmap\n");
    threadsarguments[i]->iterationsStart = &array[i*tasksPerThread];
    threadsarguments[i]->numIterations = tasksPerThread;
    printf("intparmap\n");
    threadsarguments[i]->function = function; 
  }

  threadsarguments[numofthreads-1]->iterationsStart = &array[(numofthreads-1)*tasksPerThread];
  threadsarguments[numofthreads-1]->numIterations = arrayLength-(numofthreads-1)*tasksPerThread;
  threadsarguments[numofthreads-1]->function = function; 

  int rc;
  for (int i=0; i<(numofthreads); i++) //start threads
  {
    printf("Creating thread %d\n", i);
    rc = pthread_create(&threads[i], NULL, singlethreaditerator, (void *) &threadsarguments[i]);
    assert(0 == rc);
  }

  //wait for threads
  for (int i=0; i<numofthreads; ++i) 
  {
    // block until thread i completes
    rc = pthread_join(threads[i], NULL);
    printf("Thread %d is complete\n", i);
    assert(0 == rc);
  }
}

void intmapnew(int arraya[], int arrayb[], int arrayLength, int (*function)(int) )
{
  for(int i = 0; i < arrayLength; i++)
  {
    arraya[i] = function(arrayb[i]);
  }
}

int square(int x)
{
  return x*x;
}

int main()
{
  printf("Hello World\n");

  int array[3] = {1,2,3};
  intparmap(array, 3, square);

  for(int i=0; i<3; i++)
  {
    printf("%d\n" , array[i]);
  }
  
}
