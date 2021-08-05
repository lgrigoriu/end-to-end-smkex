#include <iostream>
#include<string.h>
#include<cstring>
#include<vector>
#include <map>
#include<algorithm>
#include "smkexrecord.h"  
#include "Smkex.h"


//can also create timestamp automatically if senttimestamp is 0
SmkexT4mRecord *getSmkexT4mRecord(int64_t timestamp, uint16_t dest_len, unsigned char *dest, 
                   uint16_t expLen, unsigned char *exp, SMKEX_T4M_Type type, uint8_t version, 
                   uint16_t length, unsigned char *data){
      SmkexT4mRecord *r=new SmkexT4mRecord;
      r->version=version;
      if (timestamp==0){
        	time_t timestamp=time(NULL);
	        //cout<<ctime(&timestamp)<<"\n";

        	r->timestamp=(int64_t)timestamp;
      }
      else
        r->timestamp=timestamp; 

      r->destLen= dest_len;
      r->dest=(unsigned char *)malloc(r->destLen);
      if(r->dest==NULL) return NULL;     
      memcpy(r->dest,dest,r->destLen );
      r->senderLen=expLen;
      r->sender=(unsigned char *)malloc(r->senderLen);
      if(r->sender==NULL) return NULL;     
      memcpy(r->sender,exp,r->senderLen);
      r->type=type;
      r->length=length;
      r->data=(unsigned char *)malloc(r->length);
      if(r->data==NULL) {
        cout<<"smkexrecord.cpp : getSmkexT4mRecord: Error allocating memory for data";
        return NULL;    
      }
      memcpy(r->data, data, r->length);      
      return r;  
}

SmkexT4mRecord * getSmkexT4mRecord2(int64_t timestamp, MpString dest, 
                   MpString exp, SMKEX_T4M_Type type, uint8_t version, 
                   uint16_t length, unsigned char *data){
      
             SmkexT4mRecord *r=new SmkexT4mRecord;
      r->version=version;
      if (timestamp==0){
        	time_t timestamp=time(NULL);
	        //cout<<ctime(&timestamp)<<"\n";

        	r->timestamp=(int64_t)timestamp;
      }
      else
        r->timestamp=timestamp; 

      r->destLen= dest.length()+1;
      r->dest=(unsigned char *)malloc(r->destLen);
      if(r->dest==NULL) return NULL;     
      memcpy(r->dest,(unsigned char *) dest.c_str(),r->destLen );
      
      
      r->senderLen=exp.length()+1;
      
      
      r->sender=(unsigned char *)malloc(r->senderLen);

      //r->sender=
      if(r->sender==NULL) return NULL;     
      memcpy(r->sender,(unsigned char *)exp.c_str(),r->senderLen);
      r->type=type;
      r->length=length;
      r->data=(unsigned char *)malloc(r->length);
      if(r->data==NULL) {
        cout<<"smkexrecord.cpp : getSmkexT4mRecord: Error allocating memory for data";
        return NULL;    
      }
      memcpy(r->data, data, r->length);      
      return r;             
                       
}


SmkexT4mRecord *getRecordFromSerial(unsigned char *serialRec){
	SmkexT4mRecord *r=new SmkexT4mRecord;
	r->version= *(uint8_t *)serialRec;
	cout<<"version="<<(int)r->version<<"\n";
	r->timestamp= *(int64_t *)(serialRec +1);
	r->destLen=*(int16_t *)(serialRec+9);
	cout<<"Dest Len = "<< r->destLen  <<"\n";
	r->dest=(unsigned char *)malloc(r->destLen);
  if(r->dest==NULL){
    cout<<"smkexrecord.cpp: getRecordFromSerial: Error could not allocate memory for r->dest.\n";
    return NULL;
  }
	memcpy(r->dest,serialRec+11,r->destLen);
	cout<<"dest: "<<r->dest<<"\n";
	r->senderLen= *(int16_t *)(serialRec+11+r->destLen);
	cout<<"Sender Len = "<< r->senderLen  <<"\n";
	r->sender=(unsigned char *)malloc(r->senderLen);
  if(r->sender==NULL){
    cout<<"smkexrecord.cpp: getRecordFromSerial: Error could not allocate memory for r->sender.\n";
    return NULL;
  }
	memcpy(r->sender,serialRec+11+r->destLen+2,r->senderLen);
	cout<<"sender: "<<r->sender<<"\n";
	r->type=(SMKEX_T4M_Type) *(uint8_t *)(serialRec+ 11  +   r->destLen  + 2 +  r->senderLen);
	cout<<"type: "<<r->type<<"\n";
	r->length=*(int16_t *) (serialRec+ 11  +   r->destLen  + 2 +  r->senderLen+1);
	cout<<"length of data: "<<r->length<<"\n";
	r->data=(unsigned char *)malloc(r->length);
  if(r->data==NULL){
    cout<<"smkexrecord.cpp: getRecordFromSerial: Error could not allocate memory for r->data.\n";
    return NULL;
  }
	memcpy(r->data,serialRec+11+r->destLen+2+r->senderLen+3,r->length);
	cout<<"data as nonce or key or honce and H or H is:\n";
	Smkex::print_key(r->data,r->length);
	cout<<"\n";
  return r;
}





unsigned char * SerializeSmkexT4mRecord(unsigned int *len, const SmkexT4mRecord *rec)
{
  unsigned char* serial;
  *len = 16 + rec->destLen+rec->senderLen+rec->length; 
  
  serial = (unsigned char*)malloc(*len);
  if (serial==NULL)
  {
    *len = 0;
    return NULL;
  }

  unsigned char * cursor=serial;  
  memcpy(cursor,&(rec->version),1);
  cursor=cursor+1;
  memcpy(cursor, &(rec->timestamp), 8);
  cout<<"Verifying timestamp copy:"<< *(int64_t *)cursor <<"\n";
  cursor+=8;
  cout<<"Verifying timestamp copy again:"<< *(int64_t *)(cursor-8) <<" so that noone overwrites it\n";
  cout<<"Verifying timestamp copy yet again from the pointer serial:"<< *(int64_t *)(serial+1) <<" accessible from serial +1\n";
  memcpy(cursor, &(rec->destLen), 2);
  cursor+=2;
  memcpy(cursor, (rec->dest), rec->destLen);
  cursor+=rec->destLen;
  memcpy (cursor, &(rec->senderLen),2);
  cursor+=2;
  memcpy (cursor, rec->sender,rec->senderLen);  // TODO VERSION AND TYPE
  cursor+=rec->senderLen;
  memcpy(cursor,&(rec->type),1);
  cursor=cursor+1;
  memcpy(cursor,&(rec->length),2);
  cursor+=2;
  memcpy(cursor,rec->data,rec->length);
  cout<<"Verifying timestamp accessibility yet again at the end of the serialization from the pointer serial:"<< *(int64_t *)(serial+1) <<" accessible from serial +1\n";

  //*len= 1+4+2+rec->destLen+2+rec->senderLen+1+2+rec->length= 11 +5+rec->destLen+rec->senderLen+rec->length
  return serial;
}
    

void printRecord(const SmkexT4mRecord * r){
  cout<<"smkexrecord.cpp: printRecord:\n";
  cout<<"Smkex protocol version: "<< (int) r->version<< "\n";
  cout<<"timestamp raw as int64_t: "<< r->timestamp<< "\n";
  time_t t =(time_t) r->timestamp;
  cout<<"timestamp formatted: "<< ctime((const time_t*)&t)<< "\n";
  //cout<<r->dest;
  //std::string dest((char *)r->dest,r->destLen);
  //	std::string dest=std::string((char *)rec->dest,rec->destLen);
  cout<<"Destination of record / Receiver id: "<< r->dest<<"\n";
  std::string sender((char *)r->sender,r->senderLen);
  cout<<"Sender id: "<< r->sender<<"  with senderLen including \\0 at the end: "<< r->senderLen  <<"\n";
  cout<<"Type of record:";
  switch((int) r->type){
    case (int)handshakeH: 
      cout<<"handshakeH\n";
      break;
    case (int)handshakeKey:
      cout<<"handshakeKey\n";
      break;
    case (int)handshakeNonce:
      cout<<"handshakeNonce\n";
      break;
    case (int)handshakeNonceH:
      cout<<"handshakeNonceH\n";
      break;
    case (int) application_data:
      cout<<"application_data";
      break;
    case (int)alert:
      cout<<"alert";
      break;
    default:
      cout<<"smkexrecord.cpp::printRecord: Error: unrecognizable record type! The tyoe is "<< r-> type<< "\n";
      exit(1);

  }
  std::string data((char *)r->sender,r->senderLen);
  cout<<"Data as key or nonce or H with nonce or H:\n";
  Smkex::print_key(r->data,(int)r->length);
  cout<<"\n Done printing smkexT4mRecord\n";

}