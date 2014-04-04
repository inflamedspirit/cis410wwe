#include <stdio.h>
#include <stdlib.h>
#include <tbb/tbb.h>
<<<<<<< HEAD
#include <pthread.h> // needed for grabbing thread id, not provided by default
=======
#include <pthread.h>
>>>>>>> dd095b87a446b4f6e5c1302e0bfe33512eed9615
#include <time.h>
#define NUM_ITERS 10
#define NUM_NAMES 8
using namespace tbb;
class Sayer_of_hello{ 
    public:  
    	Sayer_of_hello(){}
        void operator()( const blocked_range<int>& r ) const {  
            for (int i=r.begin(); i!=r.end(); i++ ){  
                printf("Alien #%d says hello, they're #%d in line.\n",pthread_self(),i);
            }  
        }  
};

int main()
{
  parallel_for(blocked_range<int>(0, 10), Sayer_of_hello());
}
