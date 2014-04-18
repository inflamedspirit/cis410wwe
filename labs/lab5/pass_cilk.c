#include <cilk/cilk.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <openssl/md5.h>

#define NUM_CALLS_PER_LOOP 100
const char* chars="0123456789";
const char* HASH;

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

void hasher(long currpass, int * done){
  char passmatch[9];
  genpass(currpass,passmatch);
  //  printf("testing: %08d\n", (currpass));
  if(!(test(HASH, passmatch))){
    printf("found: %08d\n", currpass);
    *done=1;
  }
  return;
}


int main(int argc, char** argv){

  if(argc != 2){
    printf("Usage: %s <password hash>\n",argv[0]);
    return 1;
  }
  HASH = argv[1];

  int i=0;
  long currpass=0;
  int done=0;

  while(!done){
    //    cilk_for(i=0;i<NUM_CALLS_PER_LOOP;i++){ 
    for(i=0;i<NUM_CALLS_PER_LOOP;i++){ 
      cilk_spawn(hasher(currpass+i,&done));
    }
    cilk_sync;
    currpass+=NUM_CALLS_PER_LOOP;
  }

      

}


