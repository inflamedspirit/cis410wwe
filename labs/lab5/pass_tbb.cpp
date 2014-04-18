#include <stdio.h>
#include <stdlib.h>
#include <tbb/tbb.h>
#include <pthread.h> // needed for grabbing thread id, not provided by default
#include <openssl/md5.h>
#include <iostream>
#include <iomanip> //for setf setw
using namespace std;
using namespace tbb;

#define NUM_CALLS_PER_LOOP 100

const char* chars="0123456789";
const char* HASH;

// tests if a hash matches a candidate password
int test(const char* passhash, const char* passcandidate) {
    unsigned char digest[MD5_DIGEST_LENGTH];
    
    MD5((unsigned char*)passcandidate, strlen(passcandidate), digest);
    
    char mdString[34];
    mdString[33]='\0';
    for(int i=0; i<16; i++) {
        sprintf(&mdString[i*2], "%02x", (unsigned char)digest[i]);
    }
    return strncmp(passhash, mdString, strlen(passhash));
}

// maps a PIN to a string
void genpass(long passnum, char* passbuff) {
    passbuff[8]='\0';
    int charidx;
    int symcount=strlen(chars);
    for(int i=7; i>=0; i--) {
        charidx=passnum%symcount;
        passnum=passnum/symcount;
        passbuff[i]=chars[charidx];
    }
}


class Hasher{ 
  long curr;
  char * passmatch;
  bool * done;     
public:  
  Hasher(long curr_, char * passmatch_, bool * done_) : curr(curr_), passmatch(passmatch_), done(done_) {}
  long operator()( blocked_range<int> range ) const {  
    for (int i=range.begin(); i<range.end(); i++ ){  
      char passtest[9];
      genpass(curr+(long)i,passtest);
      if(!((bool)test(HASH, passtest))){// this is maybe a bit sketch?
	*done = true;
	cout << "found: " << setfill('0') << setw(8) << (curr+(long)i) << "\n";
	//strncmp(passmatch, passtest, strlen(passtest)); 
      }else{
	//	cout << " not done\n";
      }
    }  
  }
};


int main(int argc, char** argv) {
    if(argc != 2) {
        printf("Usage: %s <password hash>\n",argv[0]);
        return 1;
    }
    HASH = argv[1];
    char passmatch[9];
    long curr=0;
    bool done=false;

      while(!done){
	for(int i=0; i<NUM_CALLS_PER_LOOP; i++){
	  parallel_for(blocked_range<int>(0,NUM_CALLS_PER_LOOP), Hasher(curr,passmatch,&done));
	  curr += NUM_CALLS_PER_LOOP;
	}
      }

}


