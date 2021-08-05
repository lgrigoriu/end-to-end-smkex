#include "MpAccSettings.h"
#include "MpBaseService.h"

MpAccSettings::MpAccSettings(MpString const& serverAddress,
							 uint32_t port,
							 MpString const& userName,
							 uint32_t pjsipLogLevel,
							 mp_network_state_t networkState,
							 bool verifyServer) : serverAddress_(serverAddress),
							   					  port_(port),
												  userName_(userName),
												  pjsipLogLevel_(pjsipLogLevel),
												  networkState_(networkState),
												  verifyServer_(verifyServer) {
	MP_LOG1("Ctor");
}

MpAccSettings::~MpAccSettings() {
}

MpAccSettings& MpAccSettings::operator=(MpAccSettings const& accSettings) {
	if (this != &accSettings) {
		this->serverAddress_ = accSettings.serverAddress_;
		this->port_ = accSettings.port_;
		this->userName_ = accSettings.userName_;
	}
	MP_LOG1("Operator=");
	return *this;
}
