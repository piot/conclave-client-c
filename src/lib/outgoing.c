/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <conclave-client/client.h>
#include <conclave-client/debug.h>
#include <conclave-client/outgoing.h>
#include <conclave-serialize/serialize.h>
#include <datagram-transport/types.h>
#include <flood/out_stream.h>
#include <inttypes.h>

static int writeRoomCreate(ClvClient* self, FldOutStream* stream)
{
    CLOG_C_INFO(&self->log, "creating room request '%s'", self->createRoomOptions.name)
    clvSerializeClientOutRoomCreate(stream, self->mainUserSessionId, &self->createRoomOptions);

    return 0;
}

static int writeRoomJoin(ClvClient* self, FldOutStream* stream)
{
    CLOG_C_INFO(
        &self->log, "creating join room request roomId:%d", self->joinRoomOptions.roomIdToJoin)
    clvSerializeClientOutRoomJoin(stream, self->mainUserSessionId, &self->joinRoomOptions);

    return 0;
}

static int writeListRooms(ClvClient* self, FldOutStream* stream)
{
    CLOG_C_INFO(&self->log, "querying for rooms list applicationId:%" PRIX64 " maxReplyCount:%d",
        self->listRoomsOptions.applicationId, self->listRoomsOptions.maximumCount)
    clvSerializeClientOutListRooms(stream, self->mainUserSessionId, &self->listRoomsOptions);

    return 0;
}

/*
static int writeRoomReJoin(ClvClient* self, FldOutStream* stream)
{
    CLOG_C_INFO(&self->log, "trying to rejoin room %d (roomConnectionIndex:%hhu)",
        self->reJoinRoomOptions.roomId, self->reJoinRoomOptions.roomConnectionIndex)

    clvSerializeClientOutRoomReJoin(stream, &self->reJoinRoomOptions);
    self->waitTime = 120;

    return 0;
}
*/

static int writeLogin(ClvClient* self, FldOutStream* stream)
{
    CLOG_C_INFO(&self->log, "serialize login %" PRIx64, self->guiseUserSessionId)
    clvSerializeClientOutLogin(stream, self->nonce, self->guiseUserSessionId);

    return 0;
}

static inline int writeUsingState(ClvClient* self, FldOutStream* outStream)
{
    switch (self->state) {
    case ClvClientStateLogIn:
        return writeLogin(self, outStream);
    case ClvClientStateRoomCreate:
        return writeRoomCreate(self, outStream);
    case ClvClientStateRoomJoin:
        return writeRoomJoin(self, outStream);
        //    case ClvClientStateRoomReJoin:
        //      return writeRoomReJoin(self, outStream);
    case ClvClientStateRoomList:
        return writeListRooms(self, outStream);
    default:
        break;
    }
    return -999;
}

static inline int sendDatagramUsingState(ClvClient* self, DatagramTransportOut* transportOut)
{
    if (self->state == ClvClientStateIdle) {
        return 0;
    }
    static uint8_t buf[DATAGRAM_TRANSPORT_MAX_SIZE];

    FldOutStream outStream;
    fldOutStreamInit(&outStream, buf, DATAGRAM_TRANSPORT_MAX_SIZE);
    int result = writeUsingState(self, &outStream);
    if (result < 0) {
        if (result == -999) {
            return 0;
        }
        CLOG_SOFT_ERROR("couldnt send it")
        return result;
    }

    CLOG_C_VERBOSE(&self->log, "sending packet %zu octets", outStream.pos)
    return transportOut->send(transportOut->self, outStream.octets, outStream.pos);
}

int clvClientOutgoing(ClvClient* self, DatagramTransportOut* transportOut)
{
#if defined CLOG_LOG_ENABLED && false
    if (self->state != ClvClientStatePlaying) {
        clvClientDebugOutput(self);
    }
#endif

    int result = sendDatagramUsingState(self, transportOut);
    if (result < 0) {
        return result;
    }

    self->frame++;

    return 0;
}
