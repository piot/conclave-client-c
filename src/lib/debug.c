/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <conclave-client/client.h>
#include <conclave-client/debug.h>

const char* clvClientStateToString(ClvClientState state)
{
    switch (state) {
    case ClvClientStateLogin:
        return "login";
    case ClvClientStateIdle:
        return "idle";
    case ClvClientStateLoggedIn:
        return "logged in";
    case ClvClientStateRoomCreate:
        return "room create";
    case ClvClientStatePlaying:
        return "playing the game";
    case ClvClientStateRoomReJoin:
        return "room rejoin";
    case ClvClientStateRoomJoin:
        return "room join";
    case ClvClientStateListRooms:
        return "trying to list rooms";
    case ClvClientStateListRoomDone:
        return "list rooms complete";
    }

    return "unknown";
}

#if defined CLOG_LOG_ENABLED

void clvClientDebugOutput(const ClvClient* self)
{
    CLOG_C_INFO(&self->log, "state: %s", stateToString(self->state))
}

#endif
