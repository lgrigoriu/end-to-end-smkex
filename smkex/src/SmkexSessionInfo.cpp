#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/dh.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include<stdio.h>
#include "Smkex.h"
#include "SmkexSessionInfo.h"

//#define DH_PARAMETER_FILENAME "../smkex/dhparam.pem"
//#define PUB_KEY_LEN 256
//#define PRIV_KEY_LEN 256

// functie de verificare din curs criptografie 
#define CHECK(assertion, call_description)  \
  do {                                      \
    if (!(assertion)) {                     \
      fprintf(stderr, "(%s, %d): ",         \
        __FILE__, __LINE__);                \
      perror(call_description);             \
      exit(EXIT_FAILURE);                   \
    }                                       \
  } while(0)



// idea: create a singleton containing all session infos, for each uri, a map containing them
//? use a dictionary or include buddy uri in sessioninfo? (currently use a map)

// ? constructor could get name of file for constructing p and g as aparameter

// create public and private key using file with given name and stor in session info



SmkexSessionInfo::SmkexSessionInfo(){
    printf("SmkexSessionInfo.cpp SmkexSessionInfo-> CTOR\n");
	  ec_key= __new_key_pair();	
	  ec_group = EC_KEY_get0_group(ec_key);
    ec_local_pub_key	 = EC_KEY_get0_public_key(ec_key);
  	// const EC_POINT* ec_priv_key = EC_KEY_get0_private_key(key);
    // Compute size of public key byte representation
    size_t pub_key_size = EC_POINT_point2oct(ec_group, ec_local_pub_key,
            POINT_CONVERSION_UNCOMPRESSED, NULL, 0, NULL);  // get required size first
	  local_pub_key=(unsigned char *)malloc(pub_key_size);
	  size_t pub_key_size2=EC_POINT_point2oct(ec_group, ec_local_pub_key,
            POINT_CONVERSION_UNCOMPRESSED, local_pub_key, pub_key_size, NULL); 
    if (pub_key_size == 0) {
        fprintf(stderr, "Error: Could not compute ECDH exchange length.\n");
        errorComputingKeys=1;
       // return -1;
    }
	  else {
		  cout<< "size of public key: "<< pub_key_size <<" now printing the resulting key\n"; // iese 65, in mod consistent
		  Smkex::print_key(local_pub_key, (int)pub_key_size);  // need the char * representation of this key in order to calculate the value of H
      local_pub_key_len=(unsigned int)pub_key_size;
      local_pub_key_length=(unsigned int)pub_key_size;
     // exit(1);
	  }

    local_priv_key=(unsigned char*)malloc(PRIV_KEY_LEN); // TODO problem: can not calculate it as char * but would need this to calculate the value of H 
    remote_pub_key= (unsigned char*) malloc(pub_key_size); // do when it comes
    remote_nonce= (unsigned char *)malloc(NONCE_LEN);
    //remote_nonce=new unsigned char[NONCE_LEN];
    session_key=    (unsigned char*) calloc(64,1);      
    remote_nonce_length=NONCE_LEN;
    remote_pub_key_length=pub_key_size;    
    local_nonce_length=NONCE_LEN;
    local_nonce=new unsigned char[NONCE_LEN];
    local_nonce2=new unsigned char[NONCE_LEN];
    Smkex::makeNonce(local_nonce,local_nonce_length);
    Smkex::makeNonce(local_nonce2,local_nonce_length); // useful in case I am initiator 
    remote_nonce_length=NONCE_LEN;  
    
    status=Smkex::notConnected;
    printf("\nSmkexSessionInfo(): constructor completed successfully\n\n\n");
   
    
}

/**
 * Open file <filename>, read public Diffie-Hellman parameters P and G and store them in <pdhm>
 * in dh (Diffie-Hellman key exchange context)
 * @param filename file from which to read P and G
 */
void SmkexSessionInfo::__read_pg_from_file(const char * filename) {
    BIO * pbio;
    /* Get DH modulus and generator (P and G) */
    pbio = BIO_new_file(filename, "r");

    CHECK(pbio != NULL, "BIO_new_file");

    /* Read P and G from f  and store in the attribute of this object */
    dh = PEM_read_bio_DHparams(pbio, NULL, NULL, NULL);
    CHECK(dh != NULL, "PEM_read_bio_DHparams");
    BIO_free(pbio);
}

// free all pointers;  TODO  verify after class is finished
SmkexSessionInfo::~SmkexSessionInfo(){
    printf("SmkexSessionInfo.cpp: ~SmkexSessionInfo-> Dtor\n");
    if (dh!=0) free(dh);
    if (local_nonce!=0) free(local_nonce); //TODO figure out exactly when pointers are given memory through malloc (only then use free)
 //    if(remote_nonce!=nullptr) free(remote_nonce); // space to which this points to was likely not obtained using malloc
    if(local_pub_key!=nullptr) free(local_pub_key);
    //if(remote_pub_key!=nullptr) free(remote_pub_key);  // the memory  space to which this points was not obtained using malloc, free causes an error
    if(session_key!=nullptr)free(session_key);
    if(iv!=0) free(nullptr);
      // for debug and initial impl TODO remove if not necessary
    if(local_priv_key!=0) free(local_priv_key); 
    if(remote_pub_key!=NULL) free(remote_pub_key); 
    if(remote_nonce!=NULL) free(remote_nonce);
    if(local_nonce2!=NULL) free(local_nonce2);
    if(session_key!=NULL)  free(session_key);
    if(receivedNonceHValue!=NULL)  free(session_key);
    if(ec_local_pub_key!=NULL)  free(session_key);
    if(ec_key!=NULL)  free(session_key);
    if(ec_group!=NULL)  free(session_key);
    printf("SmkexSessionInfo.cpp: ~SmkexSessionInfo-> Dtor ending here\n");

}
// print the public and private keys from session info

