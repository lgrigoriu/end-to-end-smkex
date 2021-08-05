#ifndef __MP_PRESENCELISTMOCK_H__
#define __MP_PRESENCELISTMOCK_H__

#include "MpIPresenceList.h"
#include "MpUtils.h"
#include <vector>

class MpPresenceListMock: public MpIPresenceList {
private:
	std::vector<MpString> _users;
public:
	void insertUser(const MpString& serial);
	bool isUserOnline(MpString const& serial);
};


#endif /* __MP_PRESENCELISTMOCK_H__ */
