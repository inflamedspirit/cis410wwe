#include <time.h>
#include <cilk/cilk.h>
#include <stdio.h>
#include <stdlib.h>
int fib(int fibMe)
{
  if(fibMe<2){
    return fibMe;
  }
  int fib1m=cilk_spawn(fib(fibMe-1));
  int fib2m=cilk_spawn(fib(fibMe-2));
  //error here, need to wait for these to finish before returning.
  cilk_sync; // force wait for threads to finish
  int retMe=fib1m+fib2m;
  return retMe;
}
int main(int argc, char* argv[])
{
  if(argc!=2)
  {
    printf("Usage: %s <integer>\n",argv[0]);
    exit(1);
  }
  int fibMe=atoi(argv[1]);
  int fibVal=fib(fibMe);
  printf("Fibonacci value %d\n",fibVal);
  return 0;
}
