#include <omp.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <openssl/md5.h>

#define NUM_CALLS_PER_LOOP 100
const char* chars="0123456789";

// tests if a hash matches a candidate password
int test(const char* passhash, const char* passcandidate) {
  unsigned char digest[MD5_DIGEST_LENGTH];
  int i;
  MD5((unsigned char*)passcandidate, strlen(passcandidate), digest);

  char mdString[34];
  mdString[33]='\0';
  for(i=0; i<16; i++) {
    sprintf(&mdString[i*2], "%02x", (unsigned char)digest[i]);
  }
  return strncmp(passhash, mdString, strlen(passhash));
 }

 // maps a PIN to a string
void genpass(long passnum, char* passbuff) {
   int i;
   passbuff[8]='\0';
   int charidx;
   int symcount=strlen(chars);
   for(i=7; i>=0; i--) {
     charidx=passnum%symcount;
     passnum=passnum/symcount;
     passbuff[i]=chars[charidx];
   }
}


int main(int argc, char** argv){

  if(argc != 2){
    printf("Usage: %s <password hash>\n",argv[0]);
    return 1;
  }
  int i=0;

  long currpass=0;
  int done=0;

  while(!done){
    #pragma omp parallel for  
    for(i=0;i<NUM_CALLS_PER_LOOP;i++){
      char passmatch[9];

      //      int foo=omp_get_thread_num();
      genpass(currpass+i,passmatch);
      if(!(test(argv[1], passmatch))){
      printf("found: %08d\n", currpass+i);
      done=1;
      }
    }
    currpass+=NUM_CALLS_PER_LOOP;
  }

  //    printf("found: %s\n",passmatch);
    return 0;
}


