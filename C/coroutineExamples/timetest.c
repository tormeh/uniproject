#include <stdio.h>
#include <time.h>

int main()
{
  struct timespec tp;
  clock_gettime(CLOCK_MONOTONIC, &tp);
  printf("%ld %ld\n", tp.tv_sec, tp.tv_nsec);
  sleep(1);
  clock_gettime(CLOCK_MONOTONIC, &tp);
  printf("%ld %ld\n", tp.tv_sec, tp.tv_nsec);
  return 0;
}
