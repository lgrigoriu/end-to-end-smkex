#ifndef __MP_SMKEX_H__
#define __MP_SMKEX_H__
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
#include "MpMsgManager.h"
#include "MpIMsg.h"
#include "MpTests.h"
#include "MpLinuxOutputStream.h"
#include "SmkexSessionInfo.h"  
#include "Smkex.h"
#include "crypto.h"
#include <stdint.h>
#include <iostream>
#include<cstring>
#include <stdint.h>
#include<string>

typedef unsigned char *smkexKey;    
// TODO remove if not necessary: we currently  hardcode the type of the keys and nonces 
// as unsigned char *
//(this can be changed to someting more general by usíng a newly defined type if desired)


class Smkex: public MpIMsg{


public:
	static int keyLength;

	static MpString myself;

	/** @brief We enumerate the states of the end-to-end SMKEX state machine 
	 *  Each SessionInfo (the SessionInfo for each buddy) is in exactly one of these states at any time
	 * Each SessionInfo also contains the  information whether the current user is the initiator of the end-to-end SMKEX key exchange for the session (in case the session is lost this can help in deciding who starts again the key exchange as initiator) 
	 */

	enum state{notConnected, waitKeyNonceHDest, waitNonceHDest, waitKeyDest,
	               waitNonceInit, waitKeyInit, waitNonce2HInit, connected};
	
    /** for each buddy the current session information is stored
	 *  buddys are identified by their serial
     */

    static map<MpString, SmkexSessionInfo *> smkexKeychain;


	/**
	 * @brief Ctor
	 * @return NONE
	 */
	Smkex();
	/**
	 * @brief Dtor
	 * @return NONE
	 */
	~Smkex();

	/**
	 * @brief copies the clientId into the smkex variable needed for making smkex records
	 * @param[in] clientId 
	*/ 	

	static void setClientId(MpString clientId);
	/**
	 * @brief gets the clientId
	 * @return the clientId of the user 
	*/ 	

	static MpString getClientId();


	


	/**
     * @brief retrieves the session information for the session with the specified buddy if it exists, else returns NULL
	 * @param[in] buddy with whom the sessionInfo to be retrieved is set up if it exists
	 * @return[sessionInfo *] returns session information in case it exists and NULL otherwise
	*/
	static SmkexSessionInfo* getSessionInfo(MpString buddy);


	/** (TODO implement Liliana)
     * @brief adds the given sessionInfo to the list of SessionInfo if no SessionInfo with buddy is available; in case a SessionInfo with buddy is available, in case force==1 it deletes the previous sessionInfo and adds the given one
	 * @param[in] buddy with whom the session is being established 
	 * @param[in] sessionInfo pointer to the SessionInfo 
	 * @param[in] force if 1 add the pair to map even if another sessionInfo for buddy already exists, else return -1 and do not add the new pair
	 * @return >=1 if successful, <=0 if not successful, 2 if an existing pair was replaced, -1 if the reason for not adding the pair was that the buddy already had a session
	*/	
	static int addSessionInfo(MpString buddy,SmkexSessionInfo *sessionInfo, int force); 


	/**
     * @brief tells whether there is SessionInfo for this buddy
	 * @param[in] buddy the buddy in the current inquiry
	 * @return 1 if there is a session, 0 if there is no session
	*/
	static int hasSessionInfo(MpString buddy);

	/**
     * @brief saves the SessionInfo for this buddy aftre a change has been made to it 
	 * (not necessary if we are working directly on the SessionInfo of the buddy in the program memory)
	 * @param[in] buddy the buddy whom the Sessioninfo concerns
	 * @return 1 if the SessionInfo was stored successfully, 0 otherwise
	*/
	static int saveSessionInfo(MpString buddy);




	/**
	 * @brief starts end-to-End SMKEX key exchange as initiator with buddy 
	 * assertion: buddy is already in the buddy list if this is neccessary
	 * creates session with adequate state and iAmInitiator values, 
	 * generates and sends public key on channel 1 and nonce_init on channel 2 
	 * (in initial implementation both channels are over the unique opensips server) 
	 * changes state to waitKeyNonceHDest 
	 * @param[in] buddy the buddy with whom the key exchange for the 
	 *                  session is supposed to be initiated
	 * @return 1 if it seems successful, otherwise 0 (if something goes wrong)
	 */
	static int initSession(MpString buddy);


	/**
	 * @brief allocates memory for the key or nonce to be stored, 
	 * writes/copies the key or nonce into the allocated memory
	 * @param[in] pointer to the key or nonce to be stored
	 * @param[in] length  the length of the key or nonce  
	 * @return a pointer to the newly reserved memory where the nonce is stored
	 */
	static unsigned char * storeKeyOrNonce(unsigned char *pointer, unsigned int length);


	/**
	 * @brief sends key or nonce of specified length to buddy over the currently used SIP server 
	 * @param[in] pointer to the key or nonce to be sent
	 * @param[in] length  the length of the key or nonce
	 * @param[in] buddy the URI for the buddy to which the key or nonce is being sent
	 * @param[in] chan the channel (1 or 2) on which it is supposed to be sent 
	 * (currently unused as we are only sending over the one opensips server) 
	 * @return 1 if the sending seemed successful and 0 otherwise
	 */

	static int sendKeyOrNonce(unsigned char *pointer, unsigned int length, MpString buddy, int chan);

 	/* calculeaza H
 	 aloca memorie pentru pointerul resultH (care va fi returnat) si scrie in locul rezervat ce a fost calculat
	 scrie in locul spre care arata resultLength ce lungime are
	 */
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
	static unsigned char* calculateH(unsigned int *resultLength, const char *key1, const  char *nonce1, const  char *key2, const char * nonce2, unsigned int nonce_length, unsigned int key_length);

	/** 
	 * @brief uses the information in the session to compute and to store the session key 
	 * @param[in] session points to the session on which the work is done
	 * @param[in] buddy the buddy with whom the session is being established (used for error message in case of failure)
	 * @returns 1 on success and -1 on failure
	*/
	static int computeAndStoreSessionKey(SmkexSessionInfo *session,MpString const& buddy);

	
	/** TODO (being implemented now)
	 * @brief process a given Smkex message from a buddy (here, the state machine is implemented)
	 * @param[in] msg the message 
	 * @param[in] msgLen the length of the message
	 * @param[in] buddy the buddy where the message comes from
	 * @return 1 if all is good, 0 if not successful in processing (for example the message was not recognized as an SMKEX message)
	*/
	static int processSmkexMessageSip(unsigned char *msg, uint32_t msgLen, MpString const &buddy);

	// TODO function to encrypt a message and send the encrypted message to buddy
	unsigned char *encriptAndSendMsg(unsigned char *message, uint32_t msgLen, MpString buddy);

	// TODO function to decrypt message and return the decrypted message while writing the length of the decrypted message in decryptedMsgLen	
	unsigned char *decryptMessage(unsigned char *message, uint32_t msgLen, MpString buddy, int *decryptedMsgLen);

	/** (TODO implement)
	 * @brief processes a received encrypted message: decrypts the message and prints it out
	 * @param[in] msg the message 
	 * @param[in] msgLen the length of the message
	 * @param[in] buddy the buddy where the message comes from
	 * @return 1 if all is good, 0 if not successful in processing (for example not able to decrypt)
	*/
   	bool processEncryptedMessage(unsigned char *msg, uint32_t msgLen, MpString const& buddy);

	/**
	 *@brief calculates the value of H and sends the nonce and the calculated value to buddy
	 * to calculate H it verifies first whether the user is the initiator and uses the function calculateH
	 *@param[in] session pointer to the session with the information needed to do all that 
	 *@param[in] buddy the buddy to whom this is sent (uses function sendKeyOrNonce) 	 
	*/
	static bool sendNonceH(SmkexSessionInfo *session,MpString const& buddy);

	/**
	 *@brief returns true is H verifies and 0 if it does not verify 
	 *@param[in] session pointer to the session with the information to be verified  
	*/
	static bool verifyH(SmkexSessionInfo *session);



	/**
	 * @brief Message received callback
	 * @param[in] buddy Message sender
	 * @param[in] msg Message
	 * @param[in] msgLen Message length
	 * @return NONE
	 */
    void onMsgReceived(MpString const& buddy,  const uint8_t* msg, uint32_t msgLen);


	/**
	 * @brief Message xfer done callback
	 * @param[in] msgId Message id
	 * @param[in] xferStatus SIP xfer status
	 * @return NONE
	 */
	void onSendMsgDone(void* msgId, mp_status_t xferStatus);


	/**
	* @brief outputs a key or nonce (method used for debug purposes)
	* @param[in] key: pointer to the first character of the key or nonce
	* @param[in] key_length: the length of the key
	 * @return NONE`
	 */
	static void print_key(unsigned char *key, int key_length);

	/** @brief makes a nonce and stores it in the space pointed to by the given pointer
	 * 	@param[in] nonce: where the nonce should be stored
	 * 	@param[in] nonce_length: the length of the nonce to be created
	 *  @return NONE
	*/
	static void makeNonce(unsigned char *nonce,unsigned int nonce_length);

	/** @brief test Smkex Record
	 * tests serialization and deserialization of SmkexT4mRecord
	 * makes a record, serializes the record, 
	 * deserializes the record and checks whether it got the same data
	*/ 
	static void testRecord(MpString buddy, MpString myself, SmkexSessionInfo *session);

	/**
	 * @brief sends alert to buddy (means that the connection attempt has failed) 
	 * @param[in] buddy 
	 * @return NONE
	 * 
	*/
	static void sendAlert(MpString const& buddy);  //TODO now implement

	static void testCryptography();

};
#endif /* __MP_SMKEX_H__*/
