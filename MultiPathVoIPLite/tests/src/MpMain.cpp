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
using namespace std;

int main() {

	/* Configure logging */
	MpLinuxOutputStream linuxOutStream;
	MpService::instance()->getLogger()->setOutputStream(&linuxOutStream);

	/* Registration */
	MpRegistration regCb;
	MpUserAccount* uc = MpService::instance()->getUserAccount();
	uc->addRegCallback((MpIRegistration*) &regCb);

	MpAccSettings accSettings(MpString("195.95.167.231"),
							   8890,
							   MpString("pjsip_test1"),
							   10 /* PJSIP log level */,
							   MP_NETWORK_WIFI,
							   false);
	uc->login(accSettings);

	/* Add buddy and subscribe to presence */
	MpPresence pres;
	MpService::instance()->getBuddyList()->addPresenceCb(&pres);
	MpBuddy buddy("pjsip_test2");
	MpService::instance()->getBuddyList()->addBuddy(buddy);

	/* Configure receive message */
	MpMsg msgRcv;
	MpService::instance()->getDataMsg()->addMsgCb(&msgRcv);

	/* Send message */
	const char msgPayload[] = "Spoof";
	MpBuffer payload((uint8_t*) msgPayload, sizeof(msgPayload));
	MpMsgPayload message("pjsip_test2", payload, 1, 5, 1, /*int creationDate,*/ MP_TYPE_MESSAGE, false);

	// MpService::instance()->getAutoResend()->addMessage(message);

	/* Configure receive call */
	MpCall callRcv;
	MpService::instance()->getCallManager()->addCallCb(&callRcv);

	/* Call buddy */
	MpService::instance()->getCallManager()->callBuddy("pjsip_test2");

	std::cout << "Press q to exit...\n";

	while(std::cin.get() != 'q');

	return 0;
}

