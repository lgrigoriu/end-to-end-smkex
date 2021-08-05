#include "MpMsg.h"
#include "MpIMsg.h"
#include "MpBaseService.h"
#include "MpStatus.h"
#include <iostream>
#include<string.h>
#include<cstring>
#include "Smkex.h"
#include<vector>
#include <map>
#include<algorithm>			
#include <openssl/ecdh.h>   //de exemplu pentru create common key
#include <time.h>
#include<cstdlib>
#include"smkexrecord.h"
#include "SmkexSessionInfo.h"  
#include "Smkex.h"
#include "crypto.h"
#define THIS_TAG "Smkex"
using namespace std;

map<MpString,SmkexSessionInfo*> Smkex::smkexKeychain;
MpString Smkex::myself;

SmkexSessionInfo* Smkex::getSessionInfo(MpString buddy){
  if (Smkex::hasSessionInfo(buddy))
	return (SmkexSessionInfo *) Smkex::smkexKeychain.find(buddy)->second;
  else return NULL;
}

int Smkex::addSessionInfo(MpString buddy,SmkexSessionInfo *sessionInfo, int force){
	if(force==1){
		Smkex::smkexKeychain.insert(std::pair<MpString,SmkexSessionInfo*>(buddy,sessionInfo));
	}
	else if (!hasSessionInfo(buddy))
		Smkex::smkexKeychain.insert(std::pair<MpString,SmkexSessionInfo*>(buddy,sessionInfo));
	//TODO return values to tell more info
	return 1; // means successful
}


int Smkex::hasSessionInfo(MpString buddy){
   	if(Smkex::smkexKeychain.find(buddy)==Smkex::smkexKeychain.end())
		return 0;
	else
		return 1;
}

// TODO Smkex::deleteSessionInfo



Smkex::Smkex(){
	MpService::instance()->getLogger()->print(THIS_TAG, __FUNCTION__,
			" Smkex:Ctor");
}

Smkex::~Smkex() {
	MpService::instance()->getLogger()->print(THIS_TAG, __FUNCTION__,
			" Smkex:Dtor");
}

void Smkex::onSendMsgDone(void* msgId, mp_status_t xferStatus) {
	MpService::instance()->getLogger()->print(THIS_TAG, __FUNCTION__,
			"Message xfer done...");
	std::cout << "Message xfer status:";
	if (xferStatus == MP_MESSAGE_SENT) {
		cout << " Smkex:OK!" << std::endl;
	} else {
		std::cout << " Smkex:NOT OK!" << std::endl;
	}
}

unsigned char *Smkex::storeKeyOrNonce(unsigned char *pointer, unsigned int length){
	unsigned char *ptr=(unsigned char *) malloc(length);
	memcpy(ptr,pointer,length); // copy the key or nonce into the allocated memory region
	cout<<"Smkex::storeKeyOrNonce: stored the following key or nonce: returning a pointer to it: \n";
	print_key(ptr,length);
	return ptr;
}

int Smkex::sendKeyOrNonce(unsigned char *pointer, unsigned int length, MpString buddy, int chan){
	if(pointer==0){
		cout<<" Smkex::sendKeyOrNonce: encountered nullpointer as parameter\nreturning 0 \n\n";
		return 0; // 
	}
	cout<<"Smkex::sendKeyOrNonce: sending key or nonce:\n";
	Smkex::print_key(pointer,length);
    MpBuffer payloadSmkex((uint8_t*) pointer, length);
	MpMsgPayload messageSmkex(buddy, payloadSmkex, 1, 5, 1, /*int creationDate,*/ MP_TYPE_MESSAGE, false);
	MpService::instance()->getAutoResend()->addMessage(messageSmkex);
	return 1; // seems successful
}


void Smkex::makeNonce(unsigned char *nonce, unsigned int nonce_length){
	if(RAND_bytes(nonce,(size_t)nonce_length)==0){
		cout<<"Smkex.cpp::makeNonce: Error creating nonce...............\nExiting\n";
		exit(1);
	}
	else{
		cout<<"Smkex.cpp::makeNonce: Successfully created local nonce\n";
		Smkex::print_key(nonce,nonce_length);
		//exit(1);
	}

}


// TODO SMKEX next
// Calculate common key
// make IV
// possibly also: check compatibility of the way DH keys are created with the rest of the strategy (other program creates DH EC keys)


int Smkex::initSession(MpString buddy){ // is called now from MpMain , TODO Liliana use function makeNonce instead
	// make local nonce 
	unsigned char *nonce= (unsigned char *) malloc(SESSION_NONCE_LENGTH);
	if(RAND_bytes(nonce,SESSION_NONCE_LENGTH)==0){
		cout<<"Smkex.cpp::initSession: Error creating nonce...............\nExiting\n";
		exit(1);
	}
	else{
		cout<<"Smkex.cpp::initSession: Successfully created local nonce\n";
	}


	if (!Smkex::hasSessionInfo(buddy)){
		SmkexSessionInfo *session= new SmkexSessionInfo();
		session->iAmSessionInitiator=1;
		addSessionInfo(buddy,session,1);
		// now record local nonce and store it in the SessionInfo
		// also record value for local_nonce_length
		session->local_nonce=nonce;
		session->local_nonce_length=SESSION_NONCE_LENGTH;
		cout<<"Initiating SMKEX session: sending public key to buddy (on channel 1)\n\n";
		print_key(session->local_pub_key, session->local_pub_key_length);
		
		SmkexT4mRecord *rec= getSmkexT4mRecord2(0,buddy,Smkex::getClientId(), handshake, SMKEX_T4M_PROTOCOL_VERSION, session->local_pub_key_length,session->local_pub_key);
		printRecord(rec);
		//cout<<"Smkex::initSession: Testing serialization on actual record, and exiting now due to test\n";
		//exit(1);
		unsigned int len; // now serialize
		unsigned char *recordSerial=SerializeSmkexT4mRecord(&len,rec);
		sendKeyOrNonce(recordSerial,len, buddy,1);
		
		cout<<"\n\nNow sending nonce_init to buddy (on channel 2):\n";
		sendKeyOrNonce(session->local_nonce,session->local_nonce_length,buddy,2);
		print_key(session->local_nonce,session->local_nonce_length);
		// update status to waitKeyNonceHDest 
		session->status=waitKeyNonceHDest;
	}
	else{
		SmkexSessionInfo *session=getSessionInfo(buddy);
		session->iAmSessionInitiator=1;
		if(session->status!=waitKeyNonceHDest){
			session->local_nonce=nonce;
			session->local_nonce_length=SESSION_NONCE_LENGTH;
			cout<<"\n\n\n \nInitiating SMKEX session: sending public key to buddy (on channel 1)\n\n\n";
			SmkexT4mRecord *rec= getSmkexT4mRecord2(0,buddy,Smkex::getClientId(), handshakeKey, SMKEX_T4M_PROTOCOL_VERSION, session->local_pub_key_length,session->local_pub_key);
			printRecord(rec);
			cout<<"Smkex::initSession: Testing serialization on actual record, and exiting now due to test\n";
			//exit(1);
			unsigned int len; // now serialize
			unsigned char *recordSerial=SerializeSmkexT4mRecord(&len,rec);
			sendKeyOrNonce(recordSerial,len, buddy,1);
			//sendKeyOrNonce(session->local_pub_key,session->local_pub_key_length, buddy,1);
			cout<<"\n\n\n\n\nNow sending nonce_init to buddy (on channel 2):\n\n\n";
			free(rec);
			rec= getSmkexT4mRecord2(0,buddy,Smkex::getClientId(), handshakeNonce, SMKEX_T4M_PROTOCOL_VERSION, session->local_nonce_length,session->local_nonce);
			free(recordSerial);
			recordSerial=SerializeSmkexT4mRecord(&len,rec);
			sendKeyOrNonce(recordSerial,len, buddy,1);
			session->status=waitKeyNonceHDest;
		}
	}
	cout<<"Smkex.cpp::initSession:with buddy "<<  buddy <<" the nonce is \n";
	print_key(getSessionInfo(buddy)->local_nonce ,SESSION_NONCE_LENGTH);

	cout<<"Smkex.cpp::initSession:with buddy "<<  buddy <<" finished\n";
	//exit(1);
	return 1; // means OK
}

/* 
 * Computes the session info for SMKEX. Basically a SHA256 of key1/nonce1/key2/nonce2.
 * The result is the SHA256 char string.
 *
 * Input Parameters:
 *  @resultLength: a pointer to an integer, where the output length of the hash will be stored
 *  @key1: the sender's (EC)DH public key
 *  @nonce1: the sender's nonce
 *  @key2: the receiver's (EC)DH public key
 *  @nonce2: the receiver's nonce
 *
 * @Returns: the Hash result
 */
unsigned char *Smkex::calculateH(unsigned int *resultLength, const  char *key1, const char *nonce1, const  char *key2, const  char * nonce2, unsigned int nonce_length, unsigned int key_length){
 	unsigned char *resultH;
  const EVP_MD *md = EVP_sha256();
  EVP_MD_CTX *mdctx;
  //unsigned char md_value[EVP_MAX_MD_SIZE];

  *resultLength = 0;
  if (nonce_length <=0)
    return NULL;
  if (key_length <=0)
    return NULL;

  resultH=(unsigned char *) malloc(EVP_MAX_MD_SIZE);
  mdctx = EVP_MD_CTX_new();
  EVP_DigestInit_ex(mdctx, md, NULL);
  EVP_DigestUpdate(mdctx, nonce1, strlen(nonce1));
  EVP_DigestUpdate(mdctx, key1, strlen(key1));
  EVP_DigestUpdate(mdctx, nonce2, strlen(nonce2));
  EVP_DigestUpdate(mdctx, key2, strlen(key2));
  EVP_DigestFinal_ex(mdctx, resultH, resultLength);
  EVP_MD_CTX_free(mdctx);

  return resultH;
}



// TODO not working
int Smkex::computeAndStoreSessionKey(SmkexSessionInfo *session,MpString const& buddy){
	// EC_KEY TEST	

	// Print remote public key if it came
	print_key(session->remote_pub_key, session->remote_pub_key_len);

	// NOW RETRIEVE EC_POINT FROM remote PUBLIC KEY (using  a different created key pair)
	// STRATEGY SEE INITIAL PROGRAM

	EC_POINT * remote_pub_key_point = EC_POINT_new(session->ec_group); // allocate place for a new point for remote public key 
	int q=EC_POINT_oct2point(session->ec_group,remote_pub_key_point,session->remote_pub_key,
	(size_t) session->remote_pub_key_len,NULL);
	if (q==0){
		cout<<"Error converting public key to point\n";
		exit(1);
	}
	else{
		cout<<"success: obtained a EC_POINT from string pertaining to remote public key\n";
	}
	//TODO determine length of session key properly (find function from EC_GROUP)
	int rc = ECDH_compute_key(session->session_key, KDF_KEY_LENGTH,  
	                         remote_pub_key_point,
	                         (const EC_KEY*) session->ec_key, nist_800_kdf); 
	cout <<" Smkex::computeAndStoreSessionKey: created a secret key; it has length"<<rc<<"\n"; // it has length 64

	print_key(session->session_key, rc);
    // sugestie web de lucru cu cheile https://stackoverflow.com/questions/58832662/elliptic-curve-diffie-hellman-public-key-size

	//EC_POINT pub_key_point=EC_POINT_oct2point(
	
	// now encrypt and decrypt something
	// 	
	



	return 1;
}
//(TODO implement)
bool Smkex::processEncryptedMessage(unsigned char * msg, uint32_t msgLen, MpString const& buddy){

	return true;
}

bool Smkex::sendNonceH(SmkexSessionInfo *session,MpString const& buddy){
	unsigned int HLen=0;
	unsigned char *h;
	if(session->iAmSessionInitiator){
		cout<<"sending nonce and h as initiator\n";
		h=calculateH(&HLen,(char *)session->remote_pub_key,(char *)session->remote_nonce, (char *)session->local_pub_key,(char *)session->local_nonce2,NONCE_LEN,session->remote_pub_key_len);
	}
	else{
		cout<<"sending nonce and h as dest\n";
		h=calculateH(&HLen,(char *)session->remote_pub_key,(char *)session->remote_nonce, (char *)session->local_pub_key,(char *)session->local_nonce2,NONCE_LEN,session->remote_pub_key_len);
	}
	if(HLen==0 || h ==0)
		return false;
	cout<< "HLen is "<<HLen<<"\n";
	print_key(h, HLen); // it turns out HLen 32
	cout<<"The nonce is\n";
	print_key(session->local_nonce,NONCE_LEN);
	unsigned char * msgNonceH= (unsigned char *)malloc(NONCE_LEN+HLen);
	memcpy(msgNonceH,session->local_nonce,NONCE_LEN);
	memcpy(msgNonceH+NONCE_LEN, h, HLen);
	SmkexT4mRecord *recnh= getSmkexT4mRecord2(0,buddy,Smkex::getClientId(), handshakeNonceH, SMKEX_T4M_PROTOCOL_VERSION, NONCE_LEN+HLen,msgNonceH);
	printRecord(recnh);
	cout<<"Smkex::processMsgSip: sending nonce and h\n";
	unsigned int len; // now serialize
	unsigned char *recordSerial=SerializeSmkexT4mRecord(&len,recnh);
	sendKeyOrNonce(recordSerial,len, buddy,2);
	return true;
}

bool Smkex::verifyH(SmkexSessionInfo *session){
	unsigned int HLen;
	//if(!session->iAmSessionInitiator)
		unsigned char *h=calculateH(&HLen,(char *)session->local_pub_key,(char *)session->local_nonce, (char *)session->remote_pub_key,(char *)session->remote_nonce,NONCE_LEN,(unsigned int)session->remote_pub_key_len);
		cout<< "HLen is "<<HLen<<"\n";
		return (strncmp((char *)h,(char *)session->receivedNonceHValue+NONCE_LEN,(int)HLen));

	return true; 
}



int Smkex::processSmkexMessageSip(unsigned char * msg, uint32_t msgLen, MpString const& buddy){
	SmkexT4mRecord *rec=getRecordFromSerial((unsigned char *)(msg+1));
	if(rec==0)
		return 0; 
	printRecord(rec);
	cout<<"searching for  session info for buddy   "<<buddy<<"\n\n";
	SmkexSessionInfo *session;
	if(hasSessionInfo(buddy)){
		cout<<"we found a session info for buddy   "<<buddy<<"\n\n";
		session=getSessionInfo(buddy);
		cout<<"we retrieved  a session info for buddy   "<<buddy<<" and the local public key for him/her is:\n\n";
		print_key(session->local_pub_key,session->local_pub_key_length);
	}	
	else{
		session=new SmkexSessionInfo(); 
		// add buddy and sessioninfo to keychain, set status to not connected, and that the user is not initiator due to initialization
		session->iAmSessionInitiator=0; 
		Smkex::addSessionInfo(buddy,session,1);
	}

	cout<<"The record type is "<<rec->type<<"\n";

	switch((int)rec->type){
		case (int)handshakeKey:
			cout<<"Smkex::processSmkexMsgSip: received handshakeKey\n";
			switch(session->status){ // maybe eliminate all wrong states and send an error message first??
				case notConnected:
					{
						cout<<"Smkex::processSmkexMsgSip: received handshakeKey; status is notConnected\n";
						session->remote_pub_key= (unsigned char *) realloc(session->remote_pub_key,(size_t)rec->length);
						memcpy(session->remote_pub_key,rec->data,rec->length);
						session->remote_pub_key_length=rec->length; // TODO remove
						session->remote_pub_key_len=rec->length;
						//session->remote_pub_key=(unsigned char *) malloc(session->remote_pub_key_len);
						print_key(session->remote_pub_key, session->remote_pub_key_len);
						
						//send my public Key
						if(computeAndStoreSessionKey(session,buddy)){
							cout<<"Smkex::processSmkexMsgSip: computed session key\n";
							print_key(session->session_key,session->session_key_len);
							//exit(1);
							// now send my public key
							cout<<"sending public key\\";
							SmkexT4mRecord *rec1= getSmkexT4mRecord2(0,buddy,Smkex::getClientId(), handshakeKey, SMKEX_T4M_PROTOCOL_VERSION, session->local_pub_key_length,session->local_pub_key);
							printRecord(rec1);
							cout<<"Smkex::processMsgSip: sending public key\n";
							unsigned int len; // now serialize
							unsigned char *recordSerial=SerializeSmkexT4mRecord(&len,rec1);
							sendKeyOrNonce(recordSerial,len, buddy,1);
							session->status=waitNonceInit;
							//exit(1);x 
						}
					}
					break;
				case waitKeyNonceHDest:
					session->remote_pub_key_length=rec->length; // TODO remove
					session->remote_pub_key_len=rec->length;
					//session->remote_pub_key=(unsigned char *) malloc(session->remote_pub_key_len);
				    memcpy(session->remote_pub_key,rec->data,rec->length);
					if(computeAndStoreSessionKey(session,buddy)){
						cout<<"Smkex::processSmkexMsgSip: computed session key\n";
						print_key(session->session_key,session->session_key_len);
						// exit(1);x 

						session->status=waitNonceHDest;
						break;
					}
				case waitKeyDest:{
					session->remote_pub_key_length=rec->length; // TODO remove
					session->remote_pub_key_len=rec->length;
					//session->remote_pub_key=(unsigned char *) malloc(session->remote_pub_key_len);
					memcpy(session->remote_pub_key,rec->data,rec->length);
					if(computeAndStoreSessionKey(session,buddy)){
						cout<<"Smkex::processSmkexMsgSip: computed session key\n";
						print_key(session->session_key,session->session_key_len);
						//exit(1);x
					}
					if (!sendNonceH(session,buddy)) {cout<<"could not send nonce and H to buddy "<<buddy<<"\n";}					
					if(verifyH(session))
					  session->status=connected;
					else{
						session->status=notConnected;
						sendAlert(buddy);// send an error message (possibly start again if I am initiator)
					}
					break;
				}
				
				case connected:
					memcpy(session->remote_pub_key,rec->data,PUB_KEY_LEN);
					session->status=waitNonceInit;
					break;  // reset session and expect the other to be initiator in the next round
					// TODO discuss whether OK / could also ignore since the message is out of place, I prefer the implemented choice		
				//default:
				//	sendAlert(buddy);						

			}
			break; 
		case (int)handshakeNonce:
			cout<<"Smkex::processSmkexMsgSip: received handshakeNonce\n";
			switch(session->status){
				case waitNonceInit:{
					cout<<"Smkex::processSmkexMsgSip: received handshakeNonce\n";
					if(session->remote_nonce==0) session->remote_nonce=new unsigned char[NONCE_LEN];
					memcpy(session->remote_nonce,rec->data,NONCE_LEN); // send my nonce and a H value
					if(!sendNonceH(session,buddy)){cout<<"could not send nonce and H to buddy "<<buddy<<"\n";}
					session->status=waitNonce2HInit;
					break;
					}
				case waitKeyNonceHDest:{
					cout<<"Smkex::processSmkexMsgSip: received handshakeNonce\n";
					memcpy(session->remote_nonce,rec->data,NONCE_LEN); // store the received values and then wait for the key
					memcpy(session->receivedNonceHValue, rec->data,rec->length); // store all of it
					session->status=waitKeyDest;
					break;
					}
				case connected:{
					memcpy(session->remote_nonce,rec->data,NONCE_LEN);
					session->status=waitKeyInit; // TODO discuss
					break;
					}
				default:
					sendAlert(buddy); // TODO check again if i am initiator also do init else not
					session->status=notConnected;
				}
			break;  
		case (int)handshakeNonceH:  // correct possibilities: we are in one of the states:  waitKeyNonceHDest, waitNonce2HInit, waitNonceHDest
			// first store the received nonce and H value then in accordance with the current status 
			// and then depending on the current session status verify the H value and change states
			// nonce goes to remoteNonce
			// H goes to receivedHValue
			// todo 
			cout<<"received record of type hanshakeNonceH\n";
			session->receivedNonceHValueLen=rec->length;
			session->receivedNonceHValue=new unsigned char[rec->length];
			memcpy(session->receivedNonceHValue, rec->data,rec->length);
		
			switch(session->status){
				case  waitKeyNonceHDest: // just store, no remote public key available
					cout<<"status is waitKeyNonceHDest\n";	
					memcpy(session->remote_nonce,rec->data,NONCE_LEN); // store the received values and then wait for the key
					break;
				case waitNonceHDest: // store values and verify
					cout<<"status is NonceHDest\n";	
					sendNonceH(session,buddy);
					memcpy(session->remote_nonce,rec->data,NONCE_LEN); 
				
					if(verifyH(session)){
						session->status=connected;
						cout<<"\n\nReached status connected as initiator!\n\n\n";	
						//exit(1);
					}
					else{
						sendAlert(buddy);
						session->status=notConnected;
						cout<<"could not connect, H did not verify\n";
						//exit(1);
					}


				break;
				case waitNonce2HInit: // verify and react (go to connected or send alert and go back to start)
					cout<<"status is Nonce2HInit\n";
					memcpy(session->remote_nonce,rec->data,NONCE_LEN); // store the received values and then wait for the key
					session->receivedNonceHValue=new unsigned char[rec->length];
					memcpy(session->receivedNonceHValue, rec->data,rec->length); // store all of it
					if(verifyH(session)){
						session->status=connected;
						cout<<"\n\n\nReached status connected as dest!\n\n\n";	
						//exit(1);
					}
					else{
						sendAlert(buddy);
						cout<<"could not connect, H did not verify\n";
						//exit(1);
						session->status=notConnected;
				}
				break;
			}
			break;
		case (int)handshakeH:       // actually not possible: H always comes with a nonce (TODO maybe check again after all else is implemented)
			cout<<"received message of type handshakeH, should not be possible\n";
			exit(1);
			break;
		case (int)alert:
			session->status=notConnected;
			if (session->iAmSessionInitiator)
				Smkex::initSession(buddy);
			break;

	}


	cout<<"Smkex::processSmkexMessageSip: returning\n\n";
	//exit(1);
	return 1;
}

void Smkex::sendAlert(MpString const& buddy){
	cout<<"sending alert, connection with buddy" <<buddy <<" could not be established \n";
	unsigned char a[2]="a"; // dummy data included (esle functions would need updating)
	SmkexT4mRecord *rec= getSmkexT4mRecord2(0,buddy,Smkex::getClientId(), alert, SMKEX_T4M_PROTOCOL_VERSION,2,a); 
	printRecord(rec);
	unsigned int len; // now serialize
	unsigned char *recordSerial=SerializeSmkexT4mRecord(&len,rec);
	sendKeyOrNonce(recordSerial,len, buddy,1);
}

void Smkex::onMsgReceived(MpString const& serial, const uint8_t* msg, uint32_t msgLen) {  // aici e aplicatia layer de sus, serial este id celelalt
	
	MpService::instance()->getLogger()->print(THIS_TAG, __FUNCTION__,
											  "Smkex:Message received...");
    //exit(1);

	std::cout << "\n\n\n\n\n         Smkex: Message from:" << serial << std::endl;
	std::cout << "      Smkex: Message length:" << msgLen << std::endl;

	std::string message((char *)msg,msgLen); //msg  nu e null-terminated	extragem mesajul din ce a venit

	std::cout << "      Smkex: Message (read as char*) is exactly ----" <<message<<  "---- and its length is ---"<<msgLen<<"---\n\n\n\n\n";  

	unsigned char *msgCpy=(unsigned char *)malloc(msgLen+1);
	memcpy(msgCpy,msg,msgLen+1);
	SmkexT4mRecord *rec=getRecordFromSerial((unsigned char *)(msg+1));
	printRecord(rec);
	Smkex::processSmkexMessageSip(msgCpy,msgLen,serial);

	cout<<"returning from function onMsgReceived\n\n\n";	

	return; // we called the other function the next things are not necessary for now

}





void Smkex::print_key(unsigned char *key, int n){
   for(int k=0; k<n; k++)
        printf("%02X", key[k]);
   printf("\n");	
}


void Smkex::testRecord(const MpString buddy, MpString myself, SmkexSessionInfo *session){
	cout<<"\n\n\n\nSmkex::testRecord \n\n\n\n";
	time_t timestamp=time(NULL);
	cout<<ctime(&timestamp)<<"\n";

	int64_t timestampInt=(int64_t)timestamp;


	cout<<"Size of timestampInt: "<< sizeof(timestampInt);
	cout<<"timestamp as int:" << (int64_t)timestampInt <<"\n";
	time_t timestamp2=(time_t) (timestampInt);
	cout<<"converted from int_64 this is:"<<ctime((const time_t*)&timestamp2)<<"\n";
	// converting time back and forth works nicely

	cout <<"buddy is: " << buddy <<"\n\n";
	cout<< "my id is: "<< myself << "\n\n";

	SmkexT4mRecord * rec= new SmkexT4mRecord;	

	rec->timestamp=timestampInt;
	rec->destLen=(uint16_t)buddy.length()+1;
	rec->dest=(unsigned char *) malloc(rec->destLen);
	memcpy(rec->dest,buddy.c_str(),rec->destLen);
	std::string dest=std::string((char *)rec->dest,rec->destLen);
	cout<<"rec->dest:"<<rec->dest<< "\n";
	cout<<"destestLen="<<rec->destLen<<"\n";
	rec->senderLen= (uint16_t)myself.length()+1;  // including the \0 at the end of the string
	rec->sender=(unsigned char *)malloc(rec->senderLen);
	memcpy(rec->sender,myself.c_str(),rec->senderLen);
	cout<<"senderLen:"<<rec->senderLen<<"\n";
	rec->version=SMKEX_T4M_PROTOCOL_VERSION;
	rec->type=handshake; // to insert type here
	rec->length=session->local_pub_key_length;
	rec->data= (unsigned char *) malloc(rec->length);
	memcpy(rec->data,session->local_pub_key,rec->length);
	print_key(rec->data,(int)rec->length);
	// (record created: OK)


	unsigned int len=0;
	//make serialized record
	unsigned char *serialRec= SerializeSmkexT4mRecord(&len, rec);
	// now extract the data out of the serialized record

	SmkexT4mRecord *rec2=getRecordFromSerial(serialRec);

	printRecord(rec2);	

	

}



void Smkex::setClientId(MpString clientId){
	Smkex::myself=MpString(clientId);
}
	
	
MpString  Smkex::getClientId(){
	return myself;

};


void Smkex::testCryptography(){
	// make two keys a,d 


}