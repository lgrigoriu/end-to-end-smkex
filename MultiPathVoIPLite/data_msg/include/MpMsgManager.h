#ifndef __MP_DATA_MSG_H__
#define __MP_DATA_MSG_H__

#include "MpSingleton.h"
#include "MpStatus.h"
#include "MpIMsg.h"
#include "MpRWLock.h"
#include "MpPool.h"
#include "MpIInvitationCallback.h"
#include <stdint.h>
#include <vector>
using namespace std;

class MpMsgManager {
	friend class MpSingleton<MpMsgManager> ;
private:
	MpPool<MpIMsg> msgCb_; ///< Message callback pool
	MpPool<MpIInvitationCallback> invCb_; ///< Invitation callback pool
	/**
	 * @brief Ctor
	 * @return NONE
	 */
	MpMsgManager();
	/**
	 * @brief Dtor
	 * @return NONE
	 */
	~MpMsgManager();
	/**
	 * @brief Process invitation request
	 * @param[in] it Callback iterator
	 * @param[in] msg Invitation payload
	 * @param[in] msgLen Invitation payload length
	 * @return none
	 */
	void processInvitationRequest(vector<MpIInvitationCallback*>::iterator it,
			const uint8_t* msg, uint32_t msgLen);
	/**
	 * @brief Process invitation response
	 * @param[in] it Callback iterator
	 * @param[in] msg Invitation payload
	 * @param[in] msgLen Invitation payload length
	 * @return none
	 */
	void processInvitationResponse(vector<MpIInvitationCallback*>::iterator it,
			const uint8_t* msg, uint32_t msgLen);
	/**
	 * @brief Process invitation delete
	 * @param[in] it Callback iterator
	 * @param[in] msg Invitation payload
	 * @param[in] msgLen Invitation payload length
	 * @return none
	 */
	void processInvitationDelete(vector<MpIInvitationCallback*>::iterator it,
			const uint8_t* msg, uint32_t msgLen);
public:
	/**
	 * @brief Add message callback
	 * @param[in] mcb Message callback
	 * @return STATUS
	 */
	mp_status_t addMsgCb(MpIMsg* mcb);
	/**
	 * @brief Remove msg callback
	 * @param[in] mcb Message callback
	 * @return STATUS
	 */
	mp_status_t rmMsgCb(MpIMsg* mcb);
	/**
	 * @brief Add invitation callback
	 * @param[in] icb Invitation callback
	 * @return STATUS
	 */
	mp_status_t addInvCb(MpIInvitationCallback* icb);
	/**
	 * @brief Remove msg callback
	 * @param[in] icb Message callback
	 * @return STATUS
	 */
	mp_status_t rmInvCb(MpIInvitationCallback* icb);
	/**
	 * @brief Message sent callback
	 * @param[in] msgId Message unique ID
	 * @param[in] xferStatus SIP xfer status
	 * @return NONE
	 */
	void onMsgSent(void* msgId, pj_status_t xferStatus);

	/**
	 * @brief Message received callback
	 * @param[in] fromUri Message sender SIP URI
	 * @param[in] msgBody Message body
	 * @param[in] msgBodyLen Message body length
	 * @return NONE
	 */
	void onMsgReceived(const char* fromUri, uint8_t* msgBody,
			uint32_t msgBodyLen);
};

typedef MpSingleton<MpMsgManager> MpDMSingleton;

#endif /* __MP_DATA_MSG_H__ */
