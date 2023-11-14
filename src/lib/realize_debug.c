/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <conclave-client/network_realizer.h>
#include <conclave-client/realize_debug.h>

#if defined CLOG_LOG_ENABLED

static const char* realizeStateToString(ClvClientRealizeState state)
{
    switch (state) {
    case ClvClientRealizeStateInit:
        return "realizeInit";
    case ClvClientRealizeStateReInit:
        return "realizeReInit";
    case ClvClientRealizeStateCleared:
        return "realizeCleared";
    case ClvClientRealizeStateLoggedIn:
        return "realizeLoggedIn";
    case ClvClientRealizeStateCreateRoom:
        return "realizeCreateRoom";
    case ClvClientRealizeStateJoinRoom:
        return "realizeJoinRoom";
    case ClvClientRealizeStateListRooms:
        return "realizeListRooms";
    case ClvClientRealizeStateListRoomsDone:
        return "realizeRoomsDone";
    }

    return "unknown";
}

void clvClientRealizeDebugOutput(const ClvClientRealize* self)
{
    CLOG_C_INFO(&self->client.log, "clv client realize state: %s", realizeStateToString(self->state))
}
#endif
