#include <stdio.h>
#include <unistd.h>
#include <threads.h>

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
  for(int i = 0; i < arrayLength; i++)
  {
    array[i] = function(array[i]);
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
