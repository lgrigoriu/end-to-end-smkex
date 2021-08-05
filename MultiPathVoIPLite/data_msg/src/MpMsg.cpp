#include "MpMsg.h"
#include "MpBaseService.h"
#include "MpStatus.h"
#include <iostream>

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

void MpMsg::onMsgReceived(MpString const& serial, const uint8_t* msg, uint32_t msgLen) {
	
	MpService::instance()->getLogger()->print(THIS_TAG, __FUNCTION__,
											  "Message received...");

	std::cout << "Message from:" << serial << std::endl;
	std::cout << "Message length:" << msgLen << std::endl;
}
