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

void intparmap(int array[], int arrayLength, int (*function)(int) )
{
  int numofcpus = sysconf(_SC_NPROCESSORS_ONLN);
  int numofthreads = numofcpus;
  pthread_t threads[numofthreads];
  int thread_args[numofthreads];
  int tasksPerThread = arrayLength/numofthreads;

  int startindex = 0
  for (int i=0; i<numofthreads; i++) 
  {

    thread_args[i] = i;
    printf("In main: creating thread %d\n", i);
    rc = pthread_create(&threads[i], NULL, task_code, (void *) &thread_args[i]);
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
  intmap(array, 3, square);

  for(int i=0; i<3; i++)
  {
    printf("%d\n" , array[i]);
  }
  
}
