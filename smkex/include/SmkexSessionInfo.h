#ifndef __MP_SMKEX_SSESSION_INFO_H__
#define __MP_SMKEX_SSESSION_INFO_H__
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/dh.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include "crypto.h"
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
#define PUB_KEY_LEN 256
#define PRIV_KEY_LEN 256
#define NONCE_LEN 8
#define DH_PARAMETER_FILENAME "../smkex/dhparam.pem"

/** Purpose of this class: 
- store session information
- store information in order to provide encript and decript functionality for messages
*/


class SmkexSessionInfo{    
// TODO de verificat daca altceva decat char* este preferabil, de exemplu string, sau daca 
//pentru platform independent poate e mai bine de folosit uint8_t* 
    private:
    DH *dh; 
    //const char * PARAMETER_FILENAME="smkex/dhparam.pem"

    public:  //TODO: codat ca state machine  
        int iAmSessionInitiator=0;
       //int currentState=notConnected; // initially not connected
        //int session_confirmed=0;
        int received_remote_public_key=0;
        //int received_remote_nonce=0;
        int sent_local_pub_key=0;
        // prin autoresend mesajul este trimis pana este primit, deci nu este necesara verificare ca a fost primit mai mult decat ce face protocolul oricum

        int status=0;  
        // TODO Liliana verificare paralelism (?! nu mai stiu ce am vrut sa spun cu acest lucru )
        unsigned char *local_priv_key=0;
        unsigned char* local_nonce=0;
        unsigned char *local_nonce2=0;
	    unsigned char* remote_nonce=0;
        unsigned char *remote_nonce2=0;
	    unsigned char* local_pub_key=0;
        unsigned int local_pub_key_length;
	    unsigned char* remote_pub_key=0;
	    unsigned int remote_pub_key_length;
        unsigned int local_nonce_length;
        unsigned int remote_nonce_length;
        unsigned char* session_key=0;
        unsigned char* iv=0;
        unsigned char* receivedNonceHValue;
        unsigned int receivedNonceHValueLen;
        const EC_POINT *ec_local_pub_key; // actually we are able to retrieve this key from a sequence of unsigned chars
        const EC_KEY *ec_key; // plan is to use this as such
        const EC_GROUP *ec_group;
        unsigned int local_pub_key_len; // for ec pub key
        unsigned int remote_pub_key_len; // for ec pub keys
        unsigned int session_key_len=64;
        int errorComputingKeys=0;

        //unsigned char *receivedHValue;
        //unsigned int receivedHValueLen;
        void setStatus(int);
        int getStatus();

        SmkexSessionInfo();
        ~SmkexSessionInfo();
        /**
        * Open file <filename>, read public Diffie-Hellman parameters P and G and store them in <pdhm>
        * in dh (Diffie-Hellman key exchange context)
        * @param filename file from which to read P and G
        */
        void __read_pg_from_file(const char * filename);

}; 
#endif
