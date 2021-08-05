#include <stdio.h>
#include "MpSIPStack.h"
#include "MpUtils.h"
#include "MpBaseService.h"
#include <pjsip/sip_transport.h>
#include <iostream>

int MpSIPStack::onHold = 0;

MpSIPStack::MpSIPStack() :
		lock_(MP_SIP_STACK_LOCK), started_(false), acc_id_(-1), captureDevice(
				0), playbackDevice(0) {
	MP_LOG1("Ctor");
}
MpSIPStack::~MpSIPStack() {
}

pj_status_t MpSIPStack::setSIPGeneralConfig(uint32_t logLevel) {

	MP_LOG1("start config");

	pjsua_config cfg;
	pjsua_logging_config log_cfg;

	pjsua_config_default(&cfg);
	cfg.cb.on_reg_state = &MpSIPStack::on_reg_state;
	cfg.cb.on_buddy_state = &MpSIPStack::on_buddy_state;
	cfg.cb.on_call_media_state = &MpSIPStack::on_call_media_state;
	cfg.cb.on_call_state = &MpSIPStack::on_call_state;
	cfg.cb.on_incoming_call = &MpSIPStack::on_incoming_call;
	cfg.cb.on_pager = &MpSIPStack::on_pager;
	cfg.cb.on_pager_status = &MpSIPStack::on_pager_status;
	char mpApp[] = MP_APP;
	cfg.user_agent = pj_str(mpApp);
    cfg.use_srtp = PJMEDIA_SRTP_OPTIONAL;
    cfg.srtp_secure_signaling = 0;
    cfg.nat_type_in_sdp = 0;
	pjsua_logging_config_default(&log_cfg);
	log_cfg.console_level = logLevel; // LOG PJSIP LOG // 10 // 0
    log_cfg.cb = &MpSIPStack::on_log;
    
	pjsua_media_config media_cfg;
	pjsua_media_config_default(&media_cfg);
	media_cfg.no_vad = PJ_TRUE;

	return pjsua_init(&cfg, &log_cfg, &media_cfg);
}

pj_status_t MpSIPStack::setSIPTransportConfig(uint32_t port,
		MpString const& certPath, MpString const& privPath, bool verifyServer) {
	MP_LOG1("start transport config");

	pjsua_transport_config cfg;
	pjsua_transport_config_default(&cfg);

	// TLS settings
	cfg.tls_setting.cert_file = pj_str((char*) certPath.c_str());
	cfg.tls_setting.privkey_file = pj_str((char*) privPath.c_str());
	cfg.tls_setting.password = pj_str((char*) ""); /*Non-encrypted private key file*/

    //    cfg.port = 8899; //8899;
    
    cfg.tls_setting.verify_server = verifyServer;
    
	return pjsua_transport_create(PJSIP_TRANSPORT_TLS, &cfg, NULL);
}

pj_status_t MpSIPStack::setSIPAccountConfig(
		MpAccSettings const& accSettings) {

	MP_LOG1("start account config");

	pjsua_acc_config cfg;
	pjsua_acc_config_default(&cfg);

	MpString userName = accSettings.getUserName();
	MpString address = accSettings.getServerAddress();

	std::stringstream id;
	id << MP_SIP_SCHEMA << userName << "@" << address << ":"
			<< accSettings.getPort() << MP_SIP_TRANSPORT;
	const MpString& sId = id.str();
	cfg.id = pj_str(const_cast<char*>(sId.c_str()));

	// Setup SIP URI
	std::stringstream uri;
	uri << MP_SIP_SCHEMA << address << ":" << accSettings.getPort()
			<< MP_SIP_TRANSPORT;
	const MpString& sUri = uri.str();
	cfg.reg_uri = pj_str(const_cast<char*>(sUri.c_str()));

	// Re-register every n seconds
	//	cfg.reg_timeout = 2700;

    cfg.reg_timeout = 610; //2400
	// Notify other contacts when you are online
	cfg.publish_enabled = PJ_TRUE;
	// Do not register account automatically
	cfg.register_on_acc_add = PJ_TRUE;
	// Disable MWI
	cfg.mwi_enabled = PJ_FALSE;
	// NAT friendly
	cfg.use_rfc5626 = PJ_TRUE;
	// Keep Alive
	cfg.ka_interval = 1;

	// Add account to pjsip
	return pjsua_acc_add(&cfg, PJ_TRUE, &acc_id_);
}

pj_status_t MpSIPStack::setCodecsPriorities(mp_network_state_t networkState) {

	pjsua_codec_info codecs[32];
	uint32_t cCount = 32;
	pj_status_t pj_status = pjsua_enum_codecs(codecs, &cCount);
	if (pj_status != PJ_SUCCESS) {
		goto ret;
	}

	/*reset codec priorities to 0*/
	for (uint32_t i = 0; i < cCount; i++) {
		pj_status = pjsua_codec_set_priority(&codecs[i].codec_id, 0);
		if (pj_status != PJ_SUCCESS) {
			goto ret;
		}
	}

	/*set codec priority for current network state*/
	pj_str_t codec;
	switch (networkState) {
	case MP_NETWORK_UNREACHABLE:
		break;
		/*Set codecs for narrow band*/
	case MP_NETWORK_GPRS:
	case MP_NETWORK_EDGE:
	case MP_NETWORK_WCDMA:
		codec = pj_str((char*) "iLBC/8000/1");
		pj_status = pjsua_codec_set_priority(&codec, 255);
		if (pj_status != PJ_SUCCESS) {
			goto ret;
		}
		break;
		/*Set codecs for wide band*/
	case MP_NETWORK_WIFI:
	case MP_NETWORK_LTE:
	case MP_NETWORK_OTHER:
	default:
		codec = pj_str((char*) "PCMU/8000/1");
		pj_status = pjsua_codec_set_priority(&codec, 255);
		if (pj_status != PJ_SUCCESS) {
			goto ret;
		}

		codec = pj_str((char*) "iLBC/8000/1");
		pj_status = pjsua_codec_set_priority(&codec, 128);
		if (pj_status != PJ_SUCCESS) {
			goto ret;
		}
		break;
	}

	ret: return pj_status;
}

mp_status_t MpSIPStack::startSIPStack(MpAccSettings const& accSettings) {

	mp_status_t status;

	lock_.lock();

	MP_LOG1("startSIPStack");
	if (started_ == true) {
		MP_LOG1("SIP already started");
		status = MP_SUCCESS;
		goto ret;
	}
	/*Stop sip stack first (for safety, if it was partially started)*/
	status = stopSIPStack();
	if (status != MP_SUCCESS) {
		MP_LOG1("SIP stop error");
		goto ret;
	}

	if (pjsua_create() != PJ_SUCCESS) {
		MP_LOG1("pjsua_create() error");
		status = MP_SIP_INIT_ERR;
		goto ret;
	}

	if (setSIPGeneralConfig(accSettings.getPjsipLogLevel()) != PJ_SUCCESS) {
		MP_LOG1("pjsua_init() error");
		status = MP_SIP_INIT_ERR;
		goto ret;
	}

	if (setSIPTransportConfig(accSettings.getPort(), "/home/bchifor/pjproject-2.10/pjsip-apps/bin/pjsip_test.crt.pem",
			"/home/bchifor/pjproject-2.10/pjsip-apps/bin/pjsip_test.key.pem", accSettings.getVerifyServer()) != PJ_SUCCESS) {
		MP_LOG1("pjsua_transport_create() error");
		status = MP_SIP_INIT_ERR;
		goto ret;
	}

	if (pjsua_start() != PJ_SUCCESS) {
		MP_LOG1("pjsua_start error");
		pjsua_destroy();
		status = MP_SIP_INIT_ERR;
		goto ret;
	}

	pjsua_get_snd_dev(&captureDevice, &playbackDevice);

	if (setSIPAccountConfig(accSettings) != PJ_SUCCESS) {
		MP_LOG1("pjsua acc add error");
		status = MP_SIP_INIT_ERR;
		goto ret;
	}

	if (pjsua_acc_set_online_status(acc_id_, PJ_TRUE) != PJ_SUCCESS) {
		MP_LOG1("pjsua acc set online failed");
		status = MP_SIP_INIT_ERR;
		goto ret;
	}

	if (setCodecsPriorities(accSettings.getNetworkState()) != PJ_SUCCESS) {
		MP_LOG1("pjsua set codecs prorities error");
		status = MP_SIP_INIT_ERR;
		goto ret;
	}

	MP_LOG1("SIP started!!");
	started_ = true;

	ret: lock_.unlock();

	return status;
}

mp_status_t MpSIPStack::stopSIPStack() {

	mp_status_t status = MP_SUCCESS;

	lock_.lock();

	if (started_ == false) {
		MP_LOG1("SIP NOT started!!");
		goto ret;
	}

	MpService::instance()->getBuddyList()->reset();
    
	/*Remove account--> send empty publish. Do not return error, SIP stack will be destroyed anyway*/
	//pjsua_acc_del(acc_id_);

	if (pjsua_destroy2(PJSUA_DESTROY_NO_RX_MSG) != PJ_SUCCESS) {
		MP_LOG1("pjsua_destroy() error");
		status = MP_SIP_DESTROY_ERR;
		goto ret;
	}

	started_ = false;
	MP_LOG1("SIP destroyed!!");

	ret: lock_.unlock();

	return status;
}

void MpSIPStack::on_reg_state(pjsua_acc_id acc_id) {
	MP_LOG1("Registration state changed");

	pjsua_acc_info info;
	pjsua_acc_get_info(acc_id, &info);
	char v[80] = { 0 };
	sprintf(v, "%d", info.status);
	MP_LOG1(v);

	MP_LOG1("Notifying UserAccount...");

	if (info.status == 200 || info.status == 0) {
		MP_LOG1("Notifying SUCCESS status");
		MpService::instance()->getUserAccount()->registration_state(
				MP_SUCCESS);
	} else {
		MP_LOG1("Notifying ERR status");
		MpService::instance()->getUserAccount()->registration_state(
				MP_STATUS_ERR);
	}

}

void MpSIPStack::on_buddy_state(pjsua_buddy_id buddy_id) {
	MP_LOG1("Buddy state changed...");
	MpService::instance()->getBuddyList()->buddy_state(buddy_id);
}

mp_status_t MpSIPStack::subscribe(MpBuddy& buddy) {
	lock_.lock();
	if (!started_) {
		lock_.unlock();
		MP_LOG1("SIP stack not started!");
		return MP_GENERAL_ERR;
	}
	MP_LOG2("Subscribing for presence.Buddy ", buddy.getBuddySerial().c_str());
	MpString buddyUri = buddy.getBuddyUri();
	/*Set pjsua config*/
	pjsua_buddy_config cfg;
	pjsua_buddy_config_default(&cfg);
	/*Subscribe*/
	cfg.subscribe = PJ_TRUE;
	MP_LOG1(buddyUri.c_str());
	cfg.uri = pj_str(const_cast<char*>(buddyUri.c_str()));

	pj_status_t status = pjsua_buddy_add(&cfg, &buddy.pjsuaId_);

	MP_LOG1("Subscription done");

	lock_.unlock();

	return status == PJ_SUCCESS ? MP_SUCCESS : MP_GENERAL_ERR;
}

mp_status_t MpSIPStack::unsubscribe(pjsua_buddy_id buddy_id) {
	lock_.lock();
	if (!started_) {
		lock_.unlock();
		MP_LOG1("SIP stack not started!");
		return MP_GENERAL_ERR;
	}

	MP_LOG1("Unsubscribing for presence...");

	//	pj_status_t status = pjsua_buddy_subscribe_pres(buddy_id, PJ_FALSE);
	pj_status_t status = pjsua_buddy_del(buddy_id);
	lock_.unlock();
	return status == PJ_SUCCESS ? MP_SUCCESS : MP_GENERAL_ERR;
}

mp_status_t MpSIPStack::call(char* uri) {
	lock_.lock();
	MP_LOG1("Calling uri...");

	pj_str_t sip_uri = pj_str(uri);
    if (!pj_thread_is_registered()) {
        pj_thread_t *a_thread;
        pj_thread_desc a_thread_desc;
        char thread_name[160];
        int len = pj_ansi_snprintf(thread_name, sizeof(thread_name),
                                   "THREAD_CALL");
        thread_name[len] = '\0';
        pj_thread_register(thread_name, a_thread_desc, &a_thread);
    }
 	pj_status_t status = pjsua_call_make_call(acc_id_, &sip_uri, MP_NULL,
	MP_NULL, MP_NULL, MP_NULL);
	lock_.unlock();
	return status == PJ_SUCCESS ? MP_SUCCESS : MP_GENERAL_ERR;
}

void MpSIPStack::on_call_media_state(pjsua_call_id call_id) {
	MP_LOG1("Call media state callback...");
	pjsua_call_info c_info;

	pj_status_t status = pjsua_call_get_info(call_id, &c_info);

	if (status != PJ_SUCCESS) {
		MP_LOG1("Call get info ERR");

		return;
	}

	if (c_info.media_status == PJSUA_CALL_MEDIA_ACTIVE) {
		MP_LOG1("Connecting call to sound device...");
		pjsua_conf_connect(c_info.conf_slot, 0);
		pjsua_conf_connect(0, c_info.conf_slot);
        MpSIPStack::onHold = 0;
	} else {
		MP_LOG1("Media not active yet...");
        MpSIPStack::onHold = 1;
	}
}

void MpSIPStack::on_call_state(pjsua_call_id call_id, pjsip_event *e) {
	MP_LOG1("Call state callback...");
	/*Get call info*/
	pjsua_call_info c_info;
	pj_status_t status = pjsua_call_get_info(call_id, &c_info);
	if (status != PJ_SUCCESS) {
		MP_LOG1("Call get info ERR");
		return;
	}
	MP_LOG2("Last call status", pj_strbuf(&c_info.last_status_text));
	printf("call status code: %d\n", (int) c_info.last_status);
	MpString uri(c_info.remote_contact.ptr, c_info.remote_contact.slen);
	MpService::instance()->getCallManager()->onCallState(call_id, c_info.state,
			c_info.last_status, const_cast<char*>(uri.c_str()));
}

void MpSIPStack::on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id,
		pjsip_rx_data *rdata) {

	MP_LOG1("Incoming call...");
	/*Get call info*/
	pjsua_call_info c_info;
	pj_status_t status = pjsua_call_get_info(call_id, &c_info);
	if (status != PJ_SUCCESS) {
		MP_LOG1("Call get info ERR");
		return;
	}

	MpString uri(c_info.remote_contact.ptr, c_info.remote_contact.slen);
	MpString caller_serial = MpUtils::getSerialFromUri(uri.c_str());
    MP_LOG2("Message received from ", uri.c_str());
    MpService::instance()->getCallManager()->onIncomingCall(call_id,
			caller_serial.c_str());
    
    /* Notify peer that phone is ringing (use the pjsua function directly because we need a method which is not protected by LOCK) */
    pjsua_call_answer(call_id, MP_RING_CALL, NULL, NULL);
}

mp_status_t MpSIPStack::answerCall(pjsua_call_id call_id,
		mp_status_t call_answer_mode) {
    char dbgMsg[32] = {0};
    
    sprintf(dbgMsg, "Answer call with SIP code %d", call_answer_mode);
    
    lock_.lock();
	MP_LOG1(dbgMsg);
	pj_status_t status = pjsua_call_answer(call_id, call_answer_mode, NULL,
	NULL);
	mp_status_t ret = MP_SUCCESS;
	if (status != PJ_SUCCESS) {
		MP_LOG1("Answer call ERR");
		ret = MP_GENERAL_ERR;
	}

	lock_.unlock();
	return ret;
}

mp_status_t MpSIPStack::endCall() {
	MP_LOG1("End call...");
	lock_.lock();

	pjsua_call_hangup_all();

	lock_.unlock();
	return MP_SUCCESS;
}

void MpSIPStack::rejectCall(pjsua_call_id call_id) {
	MP_LOG1("Reject call...");
	lock_.lock();

	pjsua_call_hangup(call_id, PJSIP_SC_BUSY_HERE, NULL, NULL);

	lock_.unlock();
}

void MpSIPStack::on_pager(pjsua_call_id call_id, const pj_str_t *from,
		const pj_str_t *to, const pj_str_t *contact, const pj_str_t *mime_type,
		const pj_str_t *body) {
	MP_LOG1("New message received...");

	MpString fromUri(from->ptr, from->slen);
	MpService::instance()->getDataMsg()->onMsgReceived(fromUri.c_str(),
			(uint8_t*) body->ptr, (uint32_t) body->slen);
}

void MpSIPStack::on_pager_status(pjsua_call_id call_id, const pj_str_t *to,
		const pj_str_t *body, void *user_data, pjsip_status_code status,
		const pj_str_t *reason) {

	MP_LOG1("Message status received...");
	MpService::instance()->getMsgTransport()->onSendMsgDone(
			status == PJSIP_SC_OK ? MP_MESSAGE_SENT : MP_MESSAGE_NOT_SENT);
}

mp_status_t MpSIPStack::sendMsg(const char* serial, const uint8_t* msg,
		uint32_t msgLen, void* msgId) {

	MP_CHECK_INPUT(serial != MP_NULL, MP_INPUT_ERR);
	MP_CHECK_INPUT(msg != MP_NULL, MP_INPUT_ERR);
	MP_CHECK_INPUT(msgLen > 0, MP_INPUT_ERR);

	MP_LOG1("Sending message...");

	/*Message data*/
	pj_str_t msgData;
	msgData.ptr = (char*) msg;
	msgData.slen = msgLen;

	MpString uri =
			MpUtils::getUriFromSerial(serial,
					MpService::instance()->getUserAccount()->accSettings_.getServerAddress().c_str(),
					MpService::instance()->getUserAccount()->accSettings_.getPort());

	/*Receiver uri*/
	pj_str_t toUri = pj_str(const_cast<char*>(uri.c_str()));

	/*Account ID*/
	int accID = MpService::instance()->getSIPStack()->getAccountID();
	
	//verify account id
	if (accID < 0)
		return MP_GENERAL_ERR;

	/*Send message*/
	pj_status_t status = pjsua_im_send(accID, &toUri, MP_NULL, &msgData,
	MP_NULL, msgId);

	return status == PJ_SUCCESS ? MP_SUCCESS : MP_GENERAL_ERR;
}

bool MpSIPStack::isSipStackStarted() {
	/*lock if sip stack is currently starting or ending*/
	lock_.lock();
	bool s = started_;
	lock_.unlock();

	return s;
}

mp_status_t MpSIPStack::setRxCallLevel(bool mute) {
	int txLevel = mute ? 0 : 1;
	if (pjsua_conf_adjust_rx_level(0, txLevel) == PJ_SUCCESS)
		return MP_SUCCESS;
	else
		return MP_GENERAL_ERR;
}

mp_status_t MpSIPStack::setHoldCall(pjsua_call_id callId, bool hold) {
	if (hold) {
		if (pjsua_call_set_hold(callId, NULL) == PJ_SUCCESS)
			return MP_SUCCESS;
		else
			return MP_GENERAL_ERR;
	} else {
		if (pjsua_call_reinvite(callId, PJSUA_CALL_UNHOLD, NULL) == PJ_SUCCESS)
			return MP_SUCCESS;
		else
			return MP_GENERAL_ERR;
	}
}

int MpSIPStack::getCaptureDevice() {
	return captureDevice;
}

int MpSIPStack::getPlaybackDevice() {
	return playbackDevice;
}

void MpSIPStack::setCaptureAndPlaybackDevices(int capture_dev,
		int playback_dev) {
	captureDevice = capture_dev;
	playbackDevice = playback_dev;
}

void MpSIPStack::turnOffSoundDevices() {
	pjsua_set_no_snd_dev();
}

void MpSIPStack::turnOnSoundDevices() {
	pjsua_set_snd_dev(captureDevice, playbackDevice);
}

mp_status_t MpSIPStack::setOnline(bool online) {
    pjsua_acc_id accId;
    pj_status_t status;
    
    lock_.lock();
    
    accId = getAccountID();
    
    /* If user account ID is not valid */
    if (accId < 0) {
        lock_.unlock();
        return MP_GENERAL_ERR;
    }
    
    /* Renew registration along with the pressence status */
    if (online) {
        status = pjsua_acc_set_registration(accId, true);
        if (status != PJ_SUCCESS) {
            lock_.unlock();
            return MP_GENERAL_ERR;
        }
    }
    
    status = pjsua_acc_set_online_status(accId, online);
    
    lock_.unlock();
    
    if (status == PJ_SUCCESS)
        return MP_SUCCESS;
    
    return MP_GENERAL_ERR;
}


void MpSIPStack::on_log(int level, const char *data, int len) {
    const char delims[] = "\n";
    char* cpy = strndup(data, len);
    char *line = strtok(cpy, delims);
    
    while(line != NULL){
        MP_LOG1(line);
        line = strtok(NULL, delims);
    }
    
    free(cpy);
}
