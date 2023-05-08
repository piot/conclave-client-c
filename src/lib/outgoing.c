/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <conclave-client/client.h>
#include <conclave-client/outgoing.h>
#include <conclave-serialize/debug.h>
#include <flood/out_stream.h>

static int updateRoomCreate(ClvClient* self, FldOutStream* stream)
{
    CLOG_C_INFO(&self->log, "creating room and game request '%s' (user session:%lu)", self->createRoomOptions.name,
              self->mainUserSessionId)
    clvSerializeClientOutRoomCreate(stream, self->mainUserSessionId, &self->createRoomOptions);
    self->waitTime = 120;

    return 0;
}

static int updateRoomJoin(ClvClient* self, FldOutStream* stream)
{
    CLOG_C_INFO(&self->log, "creating join room request roomId:%d (user session:%lu)", self->joinRoomOptions.roomIdToJoin,
              self->mainUserSessionId)
    clvSerializeClientOutRoomJoin(stream, self->mainUserSessionId, &self->joinRoomOptions);
    self->waitTime = 120;

    return 0;
}

static int updateListRooms(ClvClient* self, FldOutStream* stream)
{
    CLOG_C_INFO(&self->log, "querying for rooms list applicationId:%d, maxReplyCount:%d (user session:%lu)", self->listRoomsOptions.applicationId,
              self->listRoomsOptions.maximumCount, self->mainUserSessionId)
    clvSerializeClientOutListRooms(stream, self->mainUserSessionId, &self->listRoomsOptions);
    self->waitTime = 120;

    return 0;
}

static int updateRoomReJoin(ClvClient* self, FldOutStream* stream)
{
    CLOG_C_INFO(&self->log, "trying to rejoin room %zu (participantIndex:%lu)", self->reJoinRoomOptions.roomId,
              self->reJoinRoomOptions.roomConnectionIndex)

    clvSerializeClientOutRoomReJoin(stream, &self->reJoinRoomOptions);
    self->waitTime = 120;

    return 0;
}

static int updateLogin(ClvClient* self, FldOutStream* stream)
{
    CLOG_C_INFO(&self->log,  "serialize login '%s'", self->name)
    clvSerializeClientOutLogin(stream, self->name);
    self->waitTime = 60;

    return 0;
}

#include <conclave-client/debug.h>
#include <conclave-serialize/serialize.h>

static inline int handleStreamState(ClvClient* self, FldOutStream* outStream)
{
    switch (self->state) {
        case ClvClientStateLogin:
            return updateLogin(self, outStream);
            break;
        case ClvClientStateRoomCreate:
            return updateRoomCreate(self, outStream);
            break;
        case ClvClientStateRoomJoin:
            return updateRoomJoin(self, outStream);
            break;
        case ClvClientStateRoomReJoin:
            return updateRoomReJoin(self, outStream);
            break;
        case ClvClientStateListRooms:
            return updateListRooms(self, outStream);
            break;

        default:
            CLOG_C_ERROR(&self->log, "Unknown state %d", self->state)
    }
}

static inline int handleState(ClvClient* self, MonotonicTimeMs now, UdpTransportOut* transportOut)
{
#define UDP_MAX_SIZE (1200)
    static uint8_t buf[UDP_MAX_SIZE];

    switch (self->state) {
        case ClvClientStateIdle:
        case ClvClientStateLoggedIn:
        case ClvClientStateConnected:
        case ClvClientStatePlaying:
        case ClvClientStateListRoomDone:
            return 0;
            break;

        default: {
            FldOutStream outStream;
            fldOutStreamInit(&outStream, buf, UDP_MAX_SIZE);
            int result = handleStreamState(self, &outStream);
            if (result < 0) {
                return result;
            }
            return transportOut->send(transportOut->self, outStream.octets, outStream.pos);
        }
    }
}

int clvClientOutgoing(ClvClient* self, MonotonicTimeMs now, UdpTransportOut* transportOut)
{
    int errorCode;
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

int clvClientOutAddPacket(struct ClvClient* self, int toMemberIndex, const uint8_t* octets, size_t octetCount)
{
#define UDP_MAX_SIZE (1200)
    static uint8_t buf[UDP_MAX_SIZE];
    FldOutStream outStream;
    fldOutStreamInit(&outStream, buf, UDP_MAX_SIZE);
    clvSerializeWriteCommand(&outStream, clvSerializeCmdPacket, "outaddpacket");
    clvSerializeWriteUserSessionId(&outStream, self->mainUserSessionId);
    clvSerializeWriteRoomId(&outStream, self->mainRoomId);
    clvSerializeWriteRoomConnectionIndex(&outStream, self->roomConnectionIndex);
    clvSerializeWriteRoomConnectionIndex(&outStream, toMemberIndex);
    fldOutStreamWriteUInt16(&outStream, octetCount);
    fldOutStreamWriteOctets(&outStream, octets, octetCount);
    return self->transport.send(self->transport.self, outStream.octets, outStream.pos);
}
