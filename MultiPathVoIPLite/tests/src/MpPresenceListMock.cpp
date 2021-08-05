#include "MpPresenceListMock.h"

void MpPresenceListMock::insertUser(const MpString& serial) {
	_users.push_back(serial);
}

bool MpPresenceListMock::isUserOnline(MpString const& serial) {
	std::vector<MpString>::iterator it = _users.begin();
	for(;it != _users.end();it++) {
		if((*it) == serial) {
			return true;
		}
	}
	return false;
}
