#include "MpLogger.h"
#include "MpOutputStreamFactory.h"

MpLogger::MpLogger(): os_(0) {
}

MpLogger::~MpLogger() {
	delete os_;
}

void MpLogger::print(const char* tag, const char* fnc, const char* data)
{
#if MP_DEBUG == 1

#if	MP_LOG_TO_CONSOLE == 1
	if(os_)
		os_->printToConsole(tag, fnc, data);
#endif /* MP_LOG_TO_CONSOLE */

#if	MP_LOG_TO_FILE == 1
	if(os_)
		os_->printToFile(tag, fnc, data);
#endif /* MP_LOG_TO_FILE */

#endif /* MP_DEBUG */

}
