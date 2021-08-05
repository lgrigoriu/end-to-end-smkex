#ifndef MP_ACCSETTINGS_H_
#define MP_ACCSETTINGS_H_

#include "MpBuffer.h"
#include "MpConfig.h"
#include "MpUtils.h"
#include "MpStatus.h"
#include "MpNetworkState.h"
#include <stdint.h>

class MpAccSettings {
private:
	MpString serverAddress_; ///< SIP Server address
	uint32_t port_; ///< SIP Server port
	MpString userName_; ///< SIP user name
	uint32_t pjsipLogLevel_; ///< PJSIP log level
	mp_network_state_t networkState_; ///<network state
    bool verifyServer_;
public:
	/**
	 * @brief Ctor
	 * @param[in] serverAddress SIP Server address
	 * @param[in] port SIP Server port
	 * @return NONE
	 */
	MpAccSettings(MpString const& serverAddress, uint32_t port,
			MpString const& userName, uint32_t pjsipLogLevel, mp_network_state_t networkState, bool verifyServer);
	/**
	 * @brief Dtor
	 * @return NONE
	 */
	~MpAccSettings();

	/**
	 * @brief Get SIP server address
	 * @return SIP Server address
	 */
	MpString const& getServerAddress() const {
		return serverAddress_;
	}
	/**
	 * @brief Get SIp Server port
	 * @return SIP Server port
	 */
	uint32_t getPort() const {
		return port_;
	}

	MpString const& getUserName() const {
		return userName_;
	}
	/**
	 * @brief Get PJSIP log level
	 * @return PJSIP log level
	 */
	uint32_t getPjsipLogLevel() const {
		return pjsipLogLevel_;
	}
	
	/**
	 * @brief Get Network state
	 * @return network state
	 */
	mp_network_state_t getNetworkState() const {
		return networkState_;
	}
    
    bool getVerifyServer() const {
        return verifyServer_;
    }
	
	/**
	 * @brief Operator=
	 * @param[in] accSettings Account settings
	 * @return Account settings
	 */
	MpAccSettings& operator=(MpAccSettings const& accSettings);
};

#endif /* MP_ACCSETTINGS_H_ */
