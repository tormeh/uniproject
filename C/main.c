#include <stdio.h>

int * intmap(int array[], int arrayLength, int (*function)(int) )
{
  int retarray[arrayLength];
  for(int i = 0; i < arrayLength; i++)
  {
    retarray[i] = function(array[i]);
  }
  return *retarray;
}

int square(int x)
{
  return x*x;
}

int main()
{
  printf("Hello World\n");

  int array[3] = {1,2,3};
  int *p;
  int arrayb[3];
  p = intmap(array, 3, square);

  for(int i=0; i<3; i++)
  {
    printf("%d\n" , arrayb[i]);
  }
  
}
