/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <conclave-client/client.h>

int clvClientLogin(ClvClient* self, const char* name)
{
    self->name = tc_str_dup(name);
    self->state = ClvClientStateIdle;
    self->waitTime = 0;

    return 0;
}

static bool isInStableStateAndNotInRoom(const ClvClient* self)
{
    return self->state == ClvClientStateLoggedIn || self->state == ClvClientStateListRoomDone;
}

int clvClientRoomCreate(ClvClient* self, const ClvSerializeRoomCreateOptions* roomCreate)
{
    if (!isInStableStateAndNotInRoom(self)) {
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
    CLOG_C_DEBUG(&self->log, "joining room")
    if (!isInStableStateAndNotInRoom(self)) {
        return -2;
    }
    self->joinRoomOptions.roomIdToJoin = joinOptions->roomIdToJoin;
    self->state = ClvClientStateRoomJoin;
    self->waitTime = 0;

    return 0;
}

int clvClientReJoin(ClvClient* self)
{
    if (!isInStableStateAndNotInRoom(self)) {
        return -2;
    }

    if (self->roomConnectionIndex <= 0) {
        CLOG_C_ERROR(&self->log, "can not rejoin, we don't have participants")
        //return -1;
    }
    self->state = ClvClientStateRoomReJoin;
    self->reJoinRoomOptions.roomId = self->mainRoomId;
    self->reJoinRoomOptions.roomConnectionIndex = self->roomConnectionIndex;
    self->waitTime = 0;

    return 0;
}

int clvClientListRooms(struct ClvClient* self, const ClvSerializeListRoomsOptions* options)
{
    CLOG_C_DEBUG(&self->log, "try to list rooms")
    if (self->state != ClvClientStateLoggedIn) {
        return -2;
    }
    self->listRoomsOptions.applicationId = options->applicationId;
    self->listRoomsOptions.maximumCount = options->maximumCount;
    self->state = ClvClientStateListRooms;
    self->waitTime = 0;

    return 0;
}
