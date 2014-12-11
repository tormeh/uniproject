#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define ITERATIONS 1000000

pthread_mutex_t mutex;

static void *increment(void *arg)
{
  long *counter = arg;
  for(long i=0; i<ITERATIONS; i++)
  {
    pthread_mutex_lock(&mutex);
    *counter = *counter+1;
    pthread_mutex_unlock(&mutex);
  }
  return 0;
}

static void *decrement(void *arg)
{
  long *counter = arg;
  for(long i=0; i<ITERATIONS; i++)
  {
    pthread_mutex_lock(&mutex);
    *counter = *counter-1;
    pthread_mutex_unlock(&mutex);
  }
  return 0;
}

int main(void)
{
  printf("Hello World\n");
  long *counter = malloc(sizeof(long));
  *counter = 0;
  printf("in main before threads ds.counter=%ld\n", *counter);
  
  
  pthread_t *threads = malloc(2*sizeof(pthread_t));
  pthread_mutex_t *mutex;
  
  int rc;
  rc = pthread_create(&threads[0], NULL, increment, counter);
  assert(0 == rc);
  rc = pthread_create(&threads[1], NULL, decrement, counter);
  assert(0 == rc);
  
  for (int i=0; i<2; ++i) 
  {
    // block until thread i completes
    rc = pthread_join(threads[i], NULL);
    printf("Thread number %d is complete\n", i);
    assert(0 == rc);
  }
  
  
  printf("in main after threads counter=%ld\n", *counter);
  return 0;
}
