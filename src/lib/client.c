/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <conclave-client/client.h>
#include <conclave-client/incoming.h>
#include <conclave-client/outgoing.h>
#include <datagram-transport/types.h>
#include <flood/out_stream.h>
#include <inttypes.h>
#include <secure-random/secure_random.h>

static int sendPackets(ClvClient* self)
{
    DatagramTransportOut transportOut;
    transportOut.self = self->transport.self;
    transportOut.send = self->transport.send;

    int errorCode = clvClientOutgoing(self, &transportOut);
    if (errorCode < 0) {
        return errorCode;
    }

    return 0;
}

static int sendTick(void* _self)
{
    ClvClient* self = (ClvClient*)_self;

    int result = 0;

    if (self->state != ClvClientStateIdle) {
        result = sendPackets(self);
    }

    return result;
}

void clvClientReInit(ClvClient* self, DatagramTransport* transport)
{
    self->transport = *transport;
    self->state = ClvClientStateLogIn;
    self->pingResponseOptionsVersion = 0;
    self->roomCreateVersion = 0;
    self->listRoomsOptionsVersion = 0;
    self->mainRoomId = 0;
}

int clvClientInit(ClvClient* self, const DatagramTransport* transport,
    const GuiseSerializeUserSessionId guiseUserSessionId, MonotonicTimeMs now,
    struct ImprintAllocatorWithFree* allocator, Clog log)
{
    self->log = log;
    self->state = ClvClientStateLogIn;
    self->transport = *transport;
    self->guiseUserSessionId = guiseUserSessionId;
    self->nonce = secureRandomUInt64();
    self->pingResponseOptionsVersion = 0;
    self->roomCreateVersion = 0;
    self->listRoomsOptionsVersion = 0;
    self->mainRoomId = 0;
    Clog tickLog;
    tickLog.config = log.config;
    tc_snprintf(self->timeTickLogName, 32, "%s/tick", self->log.constantPrefix);
    tickLog.constantPrefix = self->timeTickLogName;
    datagramReassemblyInit(&self->reassembly, allocator, 16 * 1024, log);
    self->allocatorWithFree = allocator;

    timeTickInit(&self->timeTick, 160, self, sendTick, now, tickLog);

    return 0;
}

void clvClientDestroy(ClvClient* self)
{
    (void)self; // TODO: destroy ClvClient
}

void clvClientDisconnect(ClvClient* self)
{
    (void)self; // TODO: disconnect ClvClient
}

int clvClientUpdate(ClvClient* self, MonotonicTimeMs now)
{
    int receiveResult = clvClientReceiveAllInUdpBuffer(self);
    if (receiveResult < 0) {
        return receiveResult;
    }

#if defined CLOG_LOG_ENABLED && false
    clvClientRealizeDebugOutput(self);
#endif
    int tickResult = timeTickUpdate(&self->timeTick, now);
    if (tickResult < 0) {
        return tickResult;
    }

    return 0;
}

int clvClientPing(ClvClient* self, uint64_t knowledge)
{
    FldOutStream outStream;

    static uint8_t buf[DATAGRAM_TRANSPORT_MAX_SIZE];

    fldOutStreamInit(&outStream, buf, DATAGRAM_TRANSPORT_MAX_SIZE);
    int result = clvSerializeClientOutPing(&outStream, self->mainUserSessionId, knowledge);
    if (result < 0) {
        CLOG_SOFT_ERROR("couldnt send it")
        return result;
    }
    CLOG_C_VERBOSE(&self->log, "sending ping packet %zu octets", outStream.pos)
    return self->transport.send(self->transport.self, outStream.octets, outStream.pos);
}

int clvClientCreateRoom(ClvClient* self, const ClvSerializeRoomCreateOptions* createRoom)
{
    if (self->mainRoomId != 0) {
        CLOG_C_NOTICE(&self->log, "already in a room, so can not create more")
        return 0;
    }
    if (self->state != ClvClientStateLoggedIn) {
        CLOG_C_NOTICE(&self->log, "must be in a logged in state to create a room")
        return 0;
    }

    self->createRoomOptions = *createRoom;
    self->state = ClvClientStateRoomCreate;

    return 0;
}

void clvClientJoinRoom(ClvClient* self, const ClvSerializeRoomJoinOptions* joinRoom)
{
    if (self->mainRoomId != 0) {
        CLOG_C_NOTICE(&self->log, "already in a room, so can not join another one")
        return;
    }

    if (self->state != ClvClientStateLoggedIn) {
        CLOG_C_NOTICE(&self->log, "must be in a logged in state to join a room")
        return;
    }

    self->joinRoomOptions = *joinRoom;
    self->state = ClvClientStateRoomJoin;
}

void clvClientListRooms(ClvClient* self, const ClvSerializeListRoomsOptions* listRooms)
{
    if (self->state != ClvClientStateLoggedIn) {
        CLOG_C_WARN(&self->log, "not stable state, so can not list rooms")
        return;
    }
    if (self->state != ClvClientStateLoggedIn) {
        CLOG_C_NOTICE(&self->log, "must be in a logged in state to list rooms")
        return;
    }

    self->listRoomsOptions = *listRooms;
    self->state = ClvClientStateRoomList;
}

int clvClientLogin(ClvClient* self, GuiseSerializeUserSessionId userSessionId)
{
    CLOG_C_VERBOSE(&self->log, "user session id %" PRIx64, userSessionId)
    self->guiseUserSessionId = userSessionId;
    self->state = ClvClientStateLogIn;

    return 0;
}
