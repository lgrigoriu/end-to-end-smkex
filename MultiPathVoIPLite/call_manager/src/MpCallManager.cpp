#include "MpCallManager.h"
#include "MpBaseService.h"
#include <iostream>

using namespace std;
#define THIS_TAG "MpCallManager"

MpCallManager::MpCallManager() :
		call_id_(-1), lastAnswerMode_(0), callCb_("MpCallManager_CALL_LOCK"), timer_(NULL), enaWatchdog(
				true) {
	MP_LOG1("Ctor");
	setRTPWatchdog();
}

MpCallManager::~MpCallManager() {
	MP_LOG1("Dtor");
}

mp_status_t MpCallManager::callBuddy(MpString const& serial) {
	MP_LOG1("Calling buddy...");
	MpString uri = MpService::instance()->getBuddyList()->getBuddyUri(serial);
    
	mp_status_t status = MpService::instance()->getSIPStack()->call(const_cast<char*>(uri.c_str()));
	
	return status;
}

void MpCallManager::addCallCb(MpICall* call) {
	MP_LOG1("Adding call callback...");
	MP_CHECK_INPUT(call != MP_NULL,);
	callCb_.addPoolData(call);
}

void MpCallManager::rmCallCb(MpICall* call) {
	MP_LOG1("Removing call callback...");
	MP_CHECK_INPUT(call != MP_NULL,);
	callCb_.rmPoolData(call);
}

void MpCallManager::executeStopTimer() {
	if (!timer_)
		return;

	timer_->stopTimer();
	bool isTimerActive = timer_->timerIsActive();
	if (isTimerActive) { // && !didTimerStop) {
		timer_->tickTimer();
		return;
	}
}
void MpCallManager::executeStartTimer() {
	timer_->startTimer(MP_CALL_TIMEOUT);
	bool isTimerActive = timer_->timerIsActive();
	if (!isTimerActive) { // if (!didTimerStart) {
		timer_->tickTimer();
		return;
	}
}

bool MpCallManager::isTimerAvailable() {
	if (!timer_) {
		MP_LOG1("Call timer is null!");
		return false;
	}
	return true;
}

void MpCallManager::reinitializeTimer() {
	if (isTimerAvailable()) {
		executeStartTimer();
	}
}

void MpCallManager::onCallState(pjsua_call_id call_id, pjsip_inv_state call_state, pjsip_status_code last_status_code, char* remote_contact_uri) {

	MP_LOG1("onCallState");

	/*If we have an active call, then reject this one*/
	if ((call_id_ != call_id) && (call_id_ != -1)
			&& pjsua_call_is_active(call_id_)) {
		MpService::instance()->getLogger()->print(THIS_TAG, __FUNCTION__,
				"Rejecting call");
		MpService::instance()->getSIPStack()->rejectCall(call_id);
		return;
	}

	callCb_.usePool();
	/*Set call id*/
	call_id_ = call_id;
	vector<MpICall*> cCbList = callCb_.getPool();
	vector<MpICall*>::iterator it;
	switch (call_state) {
		case PJSIP_INV_STATE_CALLING:
			reinitializeTimer();
			for (it = cCbList.begin(); it < cCbList.end(); ++it) {
				(*it)->call_caller_calling(last_status_code);
			}
			break;
		case PJSIP_INV_STATE_EARLY:
            /* Reset last answer mode when a new call begins */
            lastAnswerMode_ = 0;
			if (isTimerAvailable()) {
				executeStopTimer();
			}
			for (it = cCbList.begin(); it < cCbList.end(); ++it) {
				(*it)->call_caller_early(last_status_code);
			}
			break;
		case PJSIP_INV_STATE_CONNECTING:
            /* Reset last answer mode when a new conference call begins */
            lastAnswerMode_ = 0;
			reinitializeTimer();
			for (it = cCbList.begin(); it < cCbList.end(); ++it) {
				(*it)->call_u_connecting(last_status_code);
			}
			break;
		case PJSIP_INV_STATE_CONFIRMED:
			executeStopTimer();
			for (it = cCbList.begin(); it < cCbList.end(); ++it) {
				(*it)->call_u_confirmed(last_status_code);
			}
			break;
		case PJSIP_INV_STATE_DISCONNECTED:
			if (isTimerAvailable()) {
				executeStopTimer();
			}
			for (it = cCbList.begin(); it < cCbList.end(); ++it) {
				(*it)->call_u_disconnected(last_status_code);
			}
			break;
		default:
			MP_LOG1("onCallState:Invalid state");
			
			break;
	}
	callCb_.endUsePool();
}

void MpCallManager::onIncomingCall(pjsua_call_id call_id,
		const char* caller_serial) {
    MP_LOG2("Incoming call from ", caller_serial);
	/*If we have an active call, then reject this one*/
	if ((call_id_ != call_id) && (call_id_ != -1)
			&& pjsua_call_is_active(call_id_)) {
		MpService::instance()->getLogger()->print(THIS_TAG, __FUNCTION__,
				"Rejecting call");
		MpService::instance()->getSIPStack()->rejectCall(call_id);
		return;
	}
	callCb_.usePool();
	/*Set call id*/
	call_id_ = call_id;
	vector<MpICall*> cCbList = callCb_.getPool();
	vector<MpICall*>::iterator it;
	for (it = cCbList.begin(); it < cCbList.end(); ++it) {
		(*it)->call_callee_incoming(caller_serial);
	}
	callCb_.endUsePool();
	
	reinitializeTimer();
}

mp_status_t MpCallManager::answerCall(mp_status_t call_answer_mode) {
	MpService::instance()->getLogger()->print(THIS_TAG, __FUNCTION__,
			"Answer call from manager...");
    
    lastAnswerMode_ = (int) call_answer_mode;
    
	return MpService::instance()->getSIPStack()->answerCall(call_id_,
			call_answer_mode);
}

mp_status_t MpCallManager::endCall() {
	MpService::instance()->getLogger()->print(THIS_TAG, __FUNCTION__,
			"Ending call...");
    
    lastAnswerMode_ = (int)MP_REJECT_CALL;
    
	return MpService::instance()->getSIPStack()->endCall();
}

void MpCallManager::rejectCall() {
    MpService::instance()->getLogger()->print(THIS_TAG, __FUNCTION__,
                                               "Rejecting call...");
    if (!pjsua_call_is_active(call_id_))
        return;
    
    MpService::instance()->getSIPStack()->rejectCall(call_id_);
}

mp_status_t MpCallManager::muteCall() {
	MpService::instance()->getLogger()->print(THIS_TAG, __FUNCTION__,
			"Mute call...");
	return MpService::instance()->getSIPStack()->setRxCallLevel(true);
}

mp_status_t MpCallManager::unMuteCall() {
	MpService::instance()->getLogger()->print(THIS_TAG, __FUNCTION__,
			"UnMute call...");
	return MpService::instance()->getSIPStack()->setRxCallLevel(false);
}

mp_status_t MpCallManager::holdCall() {
	MpService::instance()->getLogger()->print(THIS_TAG, __FUNCTION__,
			"Hold call...");

	int capture_dev, playback_dev;
	pjsua_get_snd_dev(&capture_dev, &playback_dev);
	if (capture_dev != -1 && playback_dev != -1) {
		MpService::instance()->getSIPStack()->setCaptureAndPlaybackDevices(
				capture_dev, playback_dev);
	}

	mp_status_t status = MpService::instance()->getSIPStack()->setHoldCall(call_id_, true);
	enaWatchdog = false;
	timer_->stopTimer();
	
	return status;
}

mp_status_t MpCallManager::unHoldCall() {
	MpService::instance()->getLogger()->print(THIS_TAG, __FUNCTION__,
			"UnHold call...");

	MpService::instance()->getSIPStack()->turnOnSoundDevices();

	return MpService::instance()->getSIPStack()->setHoldCall(call_id_, false);
}

void MpCallManager::setCallTimer(MpICallTimer *timer) {
	timer_ = timer;
}

void MpCallManager::setKeyExchange(MpIKeyExchange * keyExchange) {
	/*FIXME destroy voice security ???*/
}

void MpCallManager::endWatchdogCall() {
	if (timer_ && enaWatchdog) {
		enaWatchdog = false;
		timer_->tickTimer();
	}
}

void MpCallManager::setDoCallback() {
	enaWatchdog = true;
}

void MpCallManager::setRTPWatchdog() {
	/*FIXME destroy voice security ???*/
}

unsigned int MpCallManager::getActiveCallCount() {
    return pjsua_call_get_count();
}
