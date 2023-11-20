/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <conclave-client/client.h>
#include <conclave-client/debug.h>

const char* clvClientStateToString(ClvClientState state)
{
    switch (state) {
    case ClvClientStateIdle:
        return "idle";
    case ClvClientStateLogIn:
        return "login";
    case ClvClientStateLoggedIn:
        return "logged in";
    case ClvClientStateRoomCreate:
        return "roomCreate";
    case ClvClientStateRoomList:
        return "roomList";
    case ClvClientStateRoomJoin:
        return "roomJoin";
    case ClvClientStateRoomReJoin:
        return "roomReJoin";
    }

    return "unknown";
}


#if defined CLOG_LOG_ENABLED

void clvClientDebugOutput(const ClvClient* self)
{
    CLOG_C_INFO(&self->log, "state: %s", clvClientStateToString(self->state))
}

#endif
