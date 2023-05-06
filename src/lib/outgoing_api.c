/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <conclave-client/client.h>

int clvClientLogin(ClvClient* self, const char* name)
{
    self->name = tc_str_dup(name);
    self->state = ClvClientStateLogin;
    self->waitTime = 0;

    return 0;
}

int clvClientRoomCreate(ClvClient* self, const ClvSerializeRoomCreateOptions* roomCreate)
{
    if (self->state != ClvClientStateLoggedIn) {
        return -2;
    }
    self->createRoomOptions = *roomCreate;
    self->createRoomOptions.name = tc_str_dup(roomCreate->name);
    self->state = ClvClientStateRoomCreate;
    self->waitTime = 0;

    return 0;
}

int clvClientRoomJoin(ClvClient* self, const ClvSerializeRoomJoinOptions* joinOptions)
{
    CLOG_DEBUG("joining room")
    if (self->state != ClvClientStateLoggedIn) {
        return -2;
    }
    self->joinRoomOptions.name = tc_str_dup(joinOptions->name);
    self->state = ClvClientStateRoomJoin;
    self->waitTime = 0;

    return 0;
}

int clvClientReJoin(ClvClient* self)
{
    if (self->roomConnectionIndex <= 0) {
        CLOG_ERROR("can not rejoin, we don't have participants")
        return -1;
    }
    self->state = ClvClientStateRoomReJoin;
    self->reJoinRoomOptions.roomId = self->mainRoomId;
    self->reJoinRoomOptions.roomConnectionIndex = self->roomConnectionIndex;
    self->waitTime = 0;

    return 0;
}
