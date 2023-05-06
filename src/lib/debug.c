/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <conclave-client/client.h>
#include <conclave-client/debug.h>

#if CONFIGURATION_DEBUG

static const char* stateToString(ClvClientState state)
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
        case ClvClientStateConnecting:
            return "connecting";
        case ClvClientStateConnected:
            return "connected";
        case ClvClientStateRoomJoin:
            return "room join";
    }

    return "unknown";
}

#endif

void clvClientDebugOutput(const ClvClient* self)
{
    CLOG_INFO("clvClientState: %s", stateToString(self->state))
}
