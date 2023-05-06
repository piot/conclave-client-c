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
    CLOG_INFO("creating room and game request '%s' (user session:%lu)", self->createRoomOptions.name,
              self->mainUserSessionId)
    clvSerializeClientOutRoomCreate(stream, self->mainUserSessionId, &self->createRoomOptions);
    self->waitTime = 120;

    return 0;
}

static int updateRoomJoin(ClvClient* self, FldOutStream* stream)
{
    CLOG_INFO("creating join room request '%s' (user session:%lu)", self->joinRoomOptions.name, self->mainUserSessionId)
    clvSerializeClientOutRoomJoin(stream, self->mainUserSessionId, &self->joinRoomOptions);
    self->waitTime = 120;

    return 0;
}

static int updateRoomReJoin(ClvClient* self, FldOutStream* stream)
{
    CLOG_INFO("trying to rejoin room %zu (participantIndex:%lu)", self->reJoinRoomOptions.roomId,
              self->reJoinRoomOptions.participantConnectionIndex)

    clvSerializeClientOutRoomReJoin(stream, &self->reJoinRoomOptions);
    self->waitTime = 120;

    return 0;
}

static int updateLogin(ClvClient* self, FldOutStream* stream)
{
    CLOG_INFO("serialize login '%s'", self->name)
    clvSerializeClientOutLogin(stream, self->name);
    self->waitTime = 60;

    return 0;
}

#include <conclave-client/debug.h>

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
        default:
            CLOG_ERROR("Unknown state %d", self->state)
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
    return 0;
}
