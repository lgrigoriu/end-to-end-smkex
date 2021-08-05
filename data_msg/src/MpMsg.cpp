#include "MpMsg.h"
#include "MpBaseService.h"
#include "MpStatus.h"
#include<iostream>
#include<string.h>
#include<cstring>
#include "Smkex.h"
#include "SmkexSessionInfo.h"
#define THIS_TAG "MpMsg"

MpMsg::MpMsg() {
	MpService::instance()->getLogger()->print(THIS_TAG, __FUNCTION__,
			"Ctor");
}

MpMsg::~MpMsg() {
	MpService::instance()->getLogger()->print(THIS_TAG, __FUNCTION__,
			"Dtor");
}

void MpMsg::onSendMsgDone(void* msgId, mp_status_t xferStatus) {
	MpService::instance()->getLogger()->print(THIS_TAG, __FUNCTION__,
			"Message xfer done...");
	std::cout << "Message xfer status:";
	if (xferStatus == MP_MESSAGE_SENT) {
		cout << "OK!" << std::endl;
	} else {
		std::cout << "NOT OK!" << std::endl;
	}
}

void MpMsg::onMsgReceived(MpString const& serial, const uint8_t* msg, uint32_t msgLen) {  // aici e aplicatia layer de sus, serial este id celelalt
	if(false){ // for now just stand down
	if(msgLen==257){ // it is likely a key
		cout<<"\n\n\n\n(still testing) Received possible key: it transmits (or prints) 2 zeroes more than were in the original key if starting to print with msg[0]\nPrinting from k=1 until k<msgLen\n";
		cout<<"Message length msglen is "<<msgLen<<" and thus subtracting 1 gives us the correct key length 256\n\n";
		for (unsigned int k=1; k < msgLen;k++){
			printf("%02X",msg[k]);
		}
		cout<<"\n\n";

		// TODO: URMATOARELE LUCRURI SUNT INCA in FAZA DE VERIFICARE / DEBUG

		if(Smkex::smkexKeychain.find(serial)==Smkex::smkexKeychain.end()){ // there is no session info for this buddy yet
			SmkexSessionInfo sessionInfo=SmkexSessionInfo();
			sessionInfo.remote_pub_key=(unsigned char *)(msg+1);
			sessionInfo.status=1; // received the first transmission (of the remote public key)
            sessionInfo.remote_pub_key_length=msgLen-1;
			Smkex::smkexKeychain.insert(std::pair<MpString,SmkexSessionInfo*>(serial,&sessionInfo));
			//now we send our public key


			cout<<"\n\nSMKEX key exchange: \nSending my public Key after receiving public key from "<< serial  << " \n\n";
		
			unsigned char *smkex_key=    sessionInfo.local_pub_key;
            MpBuffer payloadSmkex1((uint8_t*) smkex_key,sessionInfo.local_pub_key_length) ;
			MpMsgPayload messageSmkex0(serial, payloadSmkex1, 1, 5, 1, /*int creationDate,*/ MP_TYPE_MESSAGE, false);
			MpService::instance()->getAutoResend()->addMessage(messageSmkex0);
		}
		else if(Smkex::smkexKeychain[serial]->status==0){
			SmkexSessionInfo *sInfo=Smkex::smkexKeychain[serial];
			sInfo->remote_pub_key=(unsigned char *)(msg+1);
			sInfo->remote_pub_key_length=msgLen-1;
			sInfo->status=2;  // I have sent my public key and received the remote public key
		}


	}
	}

/*	if(Smkex::status==0){
		cout<< "\n\nthe Smkex::status is 0 checking whether it is a key\n\n";
		exit(1);
	}*/

	MpService::instance()->getLogger()->print(THIS_TAG, __FUNCTION__,
											  "Message received...");

	std::cout << "\n\n\n\n\n Message from:" << serial << std::endl;
	std::cout << "Message length:" << msgLen << std::endl;

	std::string message((char *)msg,msgLen);	//msg  nu e null-terminated	extragem mesajul din ce a venit

	std::cout << "Message is exactly ----" <<message<<  "---- and its length is ---"<<msgLen<<"---\n\n\n\n\n";  


//currently outcommented: Dorim raspuns: send confirmation: conf: I hereby confirm receiving message: 

/*
	size_t pos= message.find("This");
	if(message.find("This")!=string::npos)
		cout <<"\n\n\n found substring \"This\" in the message " << message<<" at position "<< pos<<"\n\n\n";
	else{

		cout <<"\n\n\n did not find substring \"This\" in the message " << message;

	} 	
	pos= message.find("Please start SMKEX");
	if(pos!=string::npos){
		cout <<"\n\n\n found substring \"Please start SMKEX\" in the message " << message<<" at position "<< pos<<"\n\n\n";
		//exit(1); 
	}
	else{

		cout <<"\n\n\n did not find substring \"Please start SMKEX\" in the message " << message;

	} 	
    pos= message.find("Hello");
	if(message.find("Hello")!=string::npos){
		cout <<"\n\n\n found substring \"Hello\" in the message " << message<<" at position "<< pos<<"\n\n\n";
		string restMessage= message.substr(strlen("Hello")+1, message.length() - strlen("Hello"));
		cout<<"besides \"Hello\" the message contains the following character sequence: --->"<<restMessage<<"<---\n\n\n";
		

	}
	else{

		cout <<"\n\n\n did not find substring \"Hello\" in the message " << message;

	} 	

*/

   if(message.find("xconf")!=string::npos){
	   cout<< "\n\n\n received the following confirmation message: "<< message<<"\n\n\n";
	   cout<<" and the position of xconf is "<< message.find("xconf")<<"\n\n";
	  // exit(1);
   }
/*  From previous version confirmation message for debugging outcomment for now 
    else{
       char *msgPayload = (char *)malloc(25);
	   strcpy(msgPayload, "xconf received your message: ");
	  
	   string check_message(msgPayload);
	   cout<<"\nconfirmation message sent: "<<check_message << " and the position of xconf is "<< check_message.find("xconf")<<" \n\n"
	   <<""; 
		//exit(1); 	


       MpBuffer payload((uint8_t*) msgPayload, strlen(msgPayload)+1);
	   // mesajul a venit de la serial


       MpMsgPayload message(serial, payload, 1, 5, 1, MP_TYPE_MESSAGE, false);
       MpService::instance()->getAutoResend()->addMessage(message);
   } */ 

  /* aici sau intr-un loc similar in clasa SMKEX va avea loc decriptarea mesajelor
 
	TODO
	cu cheia schimbata tot prin mesaje text 
	unde salvam datele pentru decriptare ? un MariaDB
	de unde stim ca ce am primit sunt date pentru decriptare (deducem din ordinea mesajelor, respectiv programam intr-un anumit fel) ?
	

	*/ 


     
	


}
