/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <conclave-client/client.h>
#include <conclave-client/debug.h>
#include <conclave-client/outgoing.h>
#include <conclave-serialize/serialize.h>
#include <flood/out_stream.h>
#include <inttypes.h>

static int updateRoomCreate(ClvClient* self, FldOutStream* stream)
{
    CLOG_C_INFO(&self->log, "creating room request '%s'", self->createRoomOptions.name)
    clvSerializeClientOutRoomCreate(stream, self->mainUserSessionId, &self->createRoomOptions);
    self->waitTime = 120;

    return 0;
}

static int updateRoomJoin(ClvClient* self, FldOutStream* stream)
{
    CLOG_C_INFO(
        &self->log, "creating join room request roomId:%d", self->joinRoomOptions.roomIdToJoin)
    clvSerializeClientOutRoomJoin(stream, self->mainUserSessionId, &self->joinRoomOptions);
    self->waitTime = 120;

    return 0;
}

static int updateListRooms(ClvClient* self, FldOutStream* stream)
{
    CLOG_C_INFO(&self->log, "querying for rooms list applicationId:%" PRIX64 " maxReplyCount:%d",
        self->listRoomsOptions.applicationId, self->listRoomsOptions.maximumCount)
    clvSerializeClientOutListRooms(stream, self->mainUserSessionId, &self->listRoomsOptions);
    self->waitTime = 120;

    return 0;
}

static int updateRoomReJoin(ClvClient* self, FldOutStream* stream)
{
    CLOG_C_INFO(&self->log, "trying to rejoin room %d (roomConnectionIndex:%hhu)",
        self->reJoinRoomOptions.roomId, self->reJoinRoomOptions.roomConnectionIndex)

    clvSerializeClientOutRoomReJoin(stream, &self->reJoinRoomOptions);
    self->waitTime = 120;

    return 0;
}

static int updateLogin(ClvClient* self, FldOutStream* stream)
{
    CLOG_C_INFO(&self->log, "serialize login '%s'", self->name)
    clvSerializeClientOutLogin(stream, self->nonce, self->guiseUserSessionId);
    self->waitTime = 60;

    return 0;
}

static inline int handleStreamState(ClvClient* self, FldOutStream* outStream)
{
    switch (self->state) {
    case ClvClientStateLogin:
        return updateLogin(self, outStream);
    case ClvClientStateRoomCreate:
        return updateRoomCreate(self, outStream);
    case ClvClientStateRoomJoin:
        return updateRoomJoin(self, outStream);
    case ClvClientStateRoomReJoin:
        return updateRoomReJoin(self, outStream);
    case ClvClientStateListRooms:
        return updateListRooms(self, outStream);
    case ClvClientStateIdle:
    case ClvClientStateConnecting:
    case ClvClientStateConnected:
    case ClvClientStateLoggedIn:
    case ClvClientStateListRoomDone:
    case ClvClientStatePlaying:
        break;
    }
    return 0;
}

static inline int handleState(
    ClvClient* self, MonotonicTimeMs now, DatagramTransportOut* transportOut)
{
    (void) now; // TODO: use rate limiting

#define UDP_MAX_SIZE (1200)
    static uint8_t buf[UDP_MAX_SIZE];

    switch (self->state) {
    case ClvClientStateIdle:
    case ClvClientStateLoggedIn:
    case ClvClientStateConnected:
    case ClvClientStatePlaying:
    case ClvClientStateListRoomDone:
    case ClvClientStateConnecting:
        return 0;

    case ClvClientStateLogin:
    case ClvClientStateRoomCreate:
    case ClvClientStateRoomJoin:
    case ClvClientStateRoomReJoin:
    case ClvClientStateListRooms: {
        FldOutStream outStream;
        fldOutStreamInit(&outStream, buf, UDP_MAX_SIZE);
        int result = handleStreamState(self, &outStream);
        if (result < 0) {
            CLOG_SOFT_ERROR("couldnt send it")
            return result;
        }
        CLOG_C_VERBOSE(&self->log, "sending packet %zu octets", outStream.pos)
        return transportOut->send(transportOut->self, outStream.octets, outStream.pos);
    }
    }
}

int clvClientOutgoing(ClvClient* self, MonotonicTimeMs now, DatagramTransportOut* transportOut)
{
    if (self->state != ClvClientStatePlaying) {
        clvClientDebugOutput(self);
    }

    int result = handleState(self, now, transportOut);
    if (result < 0) {
        return result;
    }

    self->frame++;

    return 0;
}
