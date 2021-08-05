#include "MpBaseService.h"
#include "MpCall.h"

#include <iostream>

#define THIS_TAG "MpCall"

MpCall::MpCall() {
	MP_LOG1("Ctor");
}

MpCall::~MpCall() {
	MP_LOG1("Dtor");
}

void MpCall::call_caller_calling(pjsip_status_code last_call_status) {
	MP_LOG1("call_caller_calling");
}

void MpCall::call_callee_incoming(const char* caller_serial) {
	MP_LOG1("call_callee_incoming");
	/*Auto answer call*/
	MpService::instance()->getCallManager()->answerCall(MP_ANSWER_CALL);
}

void MpCall::call_caller_early(pjsip_status_code last_call_status) {
	MP_LOG1("call_caller_early");
}

void MpCall::call_u_connecting(pjsip_status_code last_call_status) {
	MP_LOG1("call_u_connecting");
}

void MpCall::call_u_confirmed(pjsip_status_code last_call_status) {
	MP_LOG1("call_u_confirmed");
}

void MpCall::call_u_disconnected(pjsip_status_code last_call_status) {
	MP_LOG1("call_u_disconnected");
}
