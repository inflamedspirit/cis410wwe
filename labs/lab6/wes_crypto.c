#include <stdio.h>
#include <string.h>
#include <sys/stat.h>


int main(int argc, char* argv[]) {
  if(argc<=2)
  {
      printf("Usage: %s <fileToEncrypt> <key1> <key2> ... <key_n>\n",argv[0]);
      return 1;
  }

  // read in the keys
  int numKeys=argc-2;
  xorKey* keyList=(xorKey*)malloc(sizeof(xorKey)*numKeys); // allocate key list
  getKeys(keyList,&(argv[2]),numKeys);
  
  // read in the data to encrypt/decrypt
  off_t textLength=fsize(argv[1]); //length of our text
  FILE* rawFile=(FILE*)fopen(argv[1],"rb"); //The intel in plaintext
  char* rawData = (char*)malloc(sizeof(char)*textLength);
  fread(rawData,textLength,1,rawFile);
  fclose(rawFile);

  // Encrypt
  char* cypherText = (char*)malloc(sizeof(char)*textLength);
  encode(rawData,cypherText,keyList,textLength,numKeys);

  // Decrypt
  char* plainText = (char*)malloc(sizeof(char)*textLength);
  decode(cypherText,plainText,keyList,textLength,numKeys);

  // write out
  FILE* encryptedFile=(FILE*)fopen("encryptedOut","wb");
  FILE* decryptedFile=(FILE*)fopen("decryptedOut","wb");

  fwrite(cypherText,textLength,1,encryptedFile);
  fwrite(plainText,textLength,1,decryptedFile);

  fclose(encryptedFile);
  fclose(decryptedFile);

  // Check
  int i;
  for(i=0;i<textLength;i++) {
    if(rawData[i]!=plainText[i]) {
      printf("Encryption/Decryption is non-deterministic\n");
      i=textLength;
    }
  }
  
  return 0;

}
