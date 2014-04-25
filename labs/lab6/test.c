#include <stdio.h>
#include <omp.h>

int main()
{
  long int cipherChar=1,keyLoop=0, size=10000000000;
  int chunk=1000;
  //#pragma omp for schedule(static, size) reduction(*:cipherChar)
  //  #pragma omp for reduction(*:cipherChar) shared(cipherChar)
#pragma omp parallel for private(keyLoop) reduction(^:cipherChar) schedule(static,64)
  for(keyLoop=1;keyLoop<size;keyLoop++) {
    //      printf("thread:%d keyLoop:%d\n",omp_get_thread_num(),keyLoop);
    cipherChar = cipherChar^keyLoop;
  }
  printf("output:%d\n",cipherChar);
}
//  //  //private(keyLoop) //default(shared)
