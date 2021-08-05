#include "MpBaseService.h"
#include "MpSingleton.h"
#include "MpLogger.h"
#include "MpLock.h"
#include "MpBuffer.h"
#include "MpAccSettings.h"
#include "MpRegistration.h"
#include "MpIRegistration.h"
#include "MpBuddy.h"
#include "MpPresence.h"
#include "MpICall.h"
#include "MpCall.h"
#include "MpMsg.h"
#include "MpTests.h"
#include "MpLinuxOutputStream.h"
#include <stdint.h>
#include <iostream>
#include<cstring>
#include "MpSIPStack.h"
#include "MpBaseService.h"
#include "MpStatus.h"
#include "Smkex.h"
#include "SmkexSessionInfo.h"
#include<algorithm>

using namespace std;

//LG: on the server: figure out what users are logged python3 /usr/local/bin/opensips-cli -x mi ul_dump

int main(int argc, char *argv[]) {
  
   

	/* Configure logging */
	MpLinuxOutputStream linuxOutStream;
	MpService::instance()->getLogger()->setOutputStream(&linuxOutStream);
	
	string clientId;
	// client id  is the first argument
	if(argv[1]!=NULL)
	  clientId= argv[1];
	else
	  clientId="pjsip_test1";
	cout<< "The client id is  " << clientId<<"\n";


	char *myBuddy;

	if(argc>2){
	  myBuddy=(char *)malloc(strlen(argv[2]+1));
	  strcpy(myBuddy,argv[2]);
	}
	else{ 
       myBuddy=(char *)malloc(12);
	   strcpy(myBuddy,"pjsip_test2");
	}
	
	/* Registration */
	MpRegistration regCb;
	MpUserAccount* uc = MpService::instance()->getUserAccount();
	uc->addRegCallback((MpIRegistration*) &regCb);

	MpAccSettings accSettings(MpString("195.95.167.231"),
							   8890,
							   MpString(clientId),
							   10 /* PJSIP log level */,
							   MP_NETWORK_WIFI,
							   false);
	uc->login(accSettings);

	/* Add buddy and subscribe to presence */
	MpPresence pres;
	MpService::instance()->getBuddyList()->addPresenceCb(&pres);


	MpBuddy buddy(myBuddy);
	Smkex::setClientId(MpString(clientId));


	cout<<"Buddy serial for created buddy is "<<buddy.getBuddySerial()<<"\n\n";
	
	MpString buddySerial=buddy.getBuddySerial();
	
	MpService::instance()->getBuddyList()->addBuddy(buddy);

	SmkexSessionInfo *initialSessionInfo= new SmkexSessionInfo();  	
	printf("MpMain.cpp: successfully created SmkexSessionInfo\n");

	// add buddy and sessioninfo to keychain, set status to not connected, and that the user is not initiator (for now)
	Smkex::addSessionInfo(buddySerial, initialSessionInfo,1);
	initialSessionInfo->iAmSessionInitiator=0;
	initialSessionInfo->status=Smkex::notConnected;


	cout<<"\n\n\n Testing session info retrieval:\n\n\n";
	if(Smkex::hasSessionInfo(buddySerial)){
		cout<<"Instertion of session info seems to have been successful\n\n";
		SmkexSessionInfo* testSessionInfo=Smkex::getSessionInfo(buddySerial);
		cout<<"Retrieved local public key\n\n";
		Smkex::print_key(testSessionInfo->local_pub_key,256);
	}


	//now send the public key if it said init
     
	/* Configure receive message for MpMsg */
	MpMsg msgRcv;
	MpService::instance()->getDataMsg()->addMsgCb(&msgRcv);

	/*Configure receive message for class Smkex*/
	Smkex msgRcv2;
	MpService::instance()->getDataMsg()->addMsgCb(&msgRcv2);

	//now send the public key if there is a third argument in the command line that said init

	if(argc>3){
		if(strncmp(argv[3],"init",4)==0){

			//Smkex::testRecord(buddy.getBuddySerial(), MpString(clientId), initialSessionInfo);

			Smkex::initSession(MpString(myBuddy));
			
		}

	}

    

	/* Send some unencoded text messages: the sending is now as a comment, does not happen so that it does not interfere with the SMKEX key exchange*/
	char *msgPayload = (char *)malloc(30);
	strcpy(msgPayload, "Hello dear buddy ");
	strcat(msgPayload,myBuddy);
	const char msgPayload2[]= "This is me";
	MpBuffer payload((uint8_t*) msgPayload, strlen(msgPayload)+1);
	MpBuffer payload2((uint8_t*) msgPayload2, sizeof(msgPayload2));
	MpMsgPayload message(myBuddy, payload, 1, 5, 1, /*int creationDate,*/ MP_TYPE_MESSAGE, false);
    MpMsgPayload message2(myBuddy, payload2, 1, 5, 1, /*int creationDate,*/ MP_TYPE_MESSAGE, false);
	//cout<<"\n\n\n\n (not any more) sending message "<< (uint8_t*)msgPayload <<" to buddy "<<myBuddy<<"\n message length is " << strlen(msgPayload)<<"\n\n\n\n";	
	// The following text messages are not sent any longer (it was initially for test purposes)
	//MpService::instance()->getAutoResend()->addMessage(message); // mesaj care e doar char *, nu const char *
	//cout<<"\n\n\n\n sending message "<<(uint8_t*)msgPayload2 <<" to buddy "<<myBuddy<<"\n message length is " << sizeof(msgPayload2)<<"\n\n\n\n";
	//MpService::instance()->getAutoResend()->addMessage(message2); // trimis mesaj a functionat! 
	//MpService::instance()->getAutoResend()->addMessage(message2); // trimis 2 mesaje a functionat! 

	/* Configure receive call */
	MpCall callRcv;
	MpService::instance()->getCallManager()->addCallCb(&callRcv);

	/* Call buddy */
	// MpService::instance()->getCallManager()->callBuddy(myBuddy);

	std::cout << "Press q to exit...\n";

	while(std::cin.get() != 'q');


	// on message received confirm reception with a message

	return 0;
}

