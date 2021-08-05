#include <stdint.h>
#include <iostream>
#include<cstring>
#include <stdint.h>
#include<string>

#define SMKEX_T4M_PROTOCOL_VERSION 0x01
#include<time.h>
#include "Smkex.h"

enum SMKEX_T4M_Type : uint8_t
{
  handshakeKey,handshakeNonce, handshakeNonceH, handshakeH, 
  alert,
  application_data,handshake //  TODO remove last two
};


/** the record contains
  timestamp, dest_length, dest (receiver id), exp_length, exp (sender id), type, version, length of data, data 
*/
typedef struct SmkexT4mRecord{
  uint8_t version;
  int64_t timestamp;  // how many bytes and is it uniform across platforms? // int64_t or time_t
  uint16_t destLen; 
  unsigned char *dest;
  uint16_t senderLen;
  unsigned char *sender;  
  SMKEX_T4M_Type type; // uint8_t
  uint16_t length; // TODO u32int_t ?? this is only for the protocol, else the data can be bigger than that
  unsigned char* data;
} smkexRecord;




/**
 * @brief Initialize a SmkexT4mRecord object from the given data
 * @param[in] timestamp: timestamp
 * @param[in] dest_len: length of receiver id
 * @param[in] dest pointer to receiver id
 * @param[in] expLen length of sender id
 * @param[in] exp pointer to sender id 
 * @param[in] type type of message
 * @param[in] version version of Smkex protocol
 * @param[in] length  length of key/nonce/sent data
 * @param[in] data  pointer to the data
 * @return: pointer to the created record or 0 in case of failure
*/
SmkexT4mRecord * getSmkexT4mRecord(int64_t timestamp_, uint16_t dest_len, unsigned char *dest_, 
                   uint16_t expLen, unsigned char *exp, SMKEX_T4M_Type type, uint8_t version_, 
                   uint16_t length, unsigned char *data);


/**
 * @brief Initialize a SmkexT4mRecord object from the given data (using MpStrings as input for sender and receiver)
 * @param[in] timestamp: timestamp
 * @param[in] dest receiver 
 * @param[in] exp sender  
 * @param[in] type type of message
 * @param[in] version version of Smkex protocol
 * @param[in] length  length of key/nonce/sent data
 * @param[in] data  pointer to the data
 * @return: pointer to the created record or 0 in case of failure
*/
SmkexT4mRecord * getSmkexT4mRecord2(int64_t timestamp_, MpString dest_, MpString exp, SMKEX_T4M_Type type, uint8_t version_, 
                   uint16_t length, unsigned char *data);



/**
 * @brief Serialize a SmkexT4MRecord object * 
 * @param[in] len: length of serialized object (will be written into the space where the pointer points)
 * @param[in] rec: input record for serialization
 * @return: address of serialised object, NULL otherwise.
 */
unsigned char * SerializeSmkexT4mRecord(unsigned int *len, const SmkexT4mRecord *rec);

/**
 * @brief gets a pointer to a record from a serialized object
 * @param[in] serialRec: a pointer to the serialized object
 * @return r:a pointer to the extracted record, 0 if the record could not be made
*/
SmkexT4mRecord *getRecordFromSerial(unsigned char *serialRec);




/**
 * @brief prints information of the record 
 * @param[in] r: to the record
 * @return NONE
*/

void printRecord(const SmkexT4mRecord *r);