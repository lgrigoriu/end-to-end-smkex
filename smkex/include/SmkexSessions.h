#ifndef __MP_SMKEX_SESSIONS_H__
#define __MP_SMKEX_SESSIONS_H__

#include "Smkex.h"
#include "SmkexSessionInfo.h"

class SmkexSessions{

    // TODO when the buddy disappears or logs out need to decide whether to destroy session
    // if the buddy starts the protocol again upon reconnecting to the network what happens if a session is still exists at the buddy ()
    // how do I figure out that I need to do SMKEX again (simply because I received a key?(?))
    // upon logout send some encripted SMKEX-BYE message that is recognized at the other party so the session is deleted 
    static map<MpString,SmkexSessionInfo*> smkexKeychain;



    int sessionExists(MpString buddy); // returns 1 if there is a session and 0 if there is no session with buddy
    SmkexSessionInfo *getSessionInfo(MpString buddy); // returns a pointer to the SessionInfo for the session with buddy
    



}












#endif