/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <conclave-client/network_realizer.h>
#include <conclave-client/realize_debug.h>

const char* clvClientRealizeStateToString(ClvClientRealizeState state)
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

#if defined CLOG_LOG_ENABLED
void clvClientRealizeDebugOutput(const ClvClientRealize* self)
{
    CLOG_C_INFO(&self->log, "realize state: %s target: %s",
        clvClientRealizeStateToString(self->state),
        clvClientRealizeStateToString(self->targetState))
}
#endif
