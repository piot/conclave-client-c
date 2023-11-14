/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <conclave-client/client.h>
#include <conclave-client/incoming.h>
#include <conclave-serialize/client_in.h>
#include <conclave-serialize/debug.h>
#include <conclave-serialize/serialize.h>
#include <datagram-transport/types.h>
#include <flood/in_stream.h>
#include <imprint/allocator.h>
#include <inttypes.h>

static int onRoomCreateResponse(ClvClient* self, FldInStream* inStream)
{
    ClvSerializeRoomId roomId;
    clvSerializeReadRoomId(inStream, &roomId);

    uint8_t roomConnectionIndex;
    const int readErr = clvSerializeReadRoomConnectionIndex(inStream, &roomConnectionIndex);
    if (readErr < 0) {
        return readErr;
    }

    self->state = ClvClientStatePlaying;
    self->waitTime = 0;
    self->mainRoomId = roomId;
    self->roomConnectionIndex = roomConnectionIndex;

    CLOG_C_INFO(&self->log, "room create response. room id: %d connectionIndex: %d", roomId,
        roomConnectionIndex)

    return 0;
}

static int onRoomJoinResponse(ClvClient* self, FldInStream* inStream)
{
    ClvSerializeRoomId roomId;
    clvSerializeReadRoomId(inStream, &roomId);

    uint8_t roomConnectionIndex;
    int err = clvSerializeReadRoomConnectionIndex(inStream, &roomConnectionIndex);
    if (err < 0) {
        return err;
    }

    self->mainRoomId = roomId;
    self->roomConnectionIndex = roomConnectionIndex;

    CLOG_C_INFO(&self->log, "room join response. room id: %d. connection index %d", roomId,
        self->roomConnectionIndex)

    self->state = ClvClientStatePlaying;

    self->reJoinRoomOptions.roomId = roomId;
    self->reJoinRoomOptions.roomConnectionIndex = self->roomConnectionIndex;

    return 0;
}

static int onListRoomsResponse(ClvClient* self, FldInStream* inStream)
{
    clvSerializeClientInListRoomsResponse(inStream, &self->listRoomsResponseOptions);

    CLOG_C_INFO(
        &self->log, "got list of rooms back %zu", self->listRoomsResponseOptions.roomInfoCount)
    for (size_t i = 0; i < self->listRoomsResponseOptions.roomInfoCount; ++i) {
        CLOG_EXECUTE(
            const ClvSerializeRoomInfo* roomInfo = &self->listRoomsResponseOptions.roomInfos[i];)
        CLOG_C_INFO(&self->log, " %zu: %d '%s' user: %" PRIX64, i, roomInfo->roomId,
            roomInfo->roomName, roomInfo->ownerUserId)
    }

    self->state = ClvClientStateListRoomDone;
    self->waitTime = 160;

    return 0;
}

static int onRoomReJoinResponse(ClvClient* self, FldInStream* inStream)
{
    ClvSerializeRoomId roomId;
    clvSerializeReadRoomId(inStream, &roomId);
    uint8_t roomConnectionIndex;
    clvSerializeReadRoomConnectionIndex(inStream, &roomConnectionIndex);

    CLOG_C_VERBOSE(&self->log, "rejoined room: %d %d", roomId, roomConnectionIndex)

    self->mainRoomId = roomId;
    self->roomConnectionIndex = roomConnectionIndex;
    self->state = ClvClientStatePlaying;
    self->waitTime = 0;

    return 0;
}

static int onLoginResponse(ClvClient* self, FldInStream* inStream)
{
    ClvSerializeClientNonce toClientNonce;
    ClvSerializeUserSessionId userSessionId;

    clvSerializeClientInLogin(inStream, &toClientNonce, &userSessionId);

    if (toClientNonce != self->nonce) {
        CLOG_C_NOTICE(&self->log,
            "received wrong nonce. expected %" PRIX64 " but received %" PRIX64, self->nonce,
            toClientNonce)
        return 0;
    }

    CLOG_C_INFO(&self->log, "Logged in as session %" PRIX64, userSessionId)

    self->mainUserSessionId = userSessionId;
    self->state = ClvClientStateLoggedIn;

    return 0;
}

static int clvClientFeed(ClvClient* self, const uint8_t* data, size_t len)
{
    FldInStream inStream;
    fldInStreamInit(&inStream, data, len);

    uint8_t cmd;
    fldInStreamReadUInt8(&inStream, &cmd);
#if defined CLOG_LOG_ENABLED
    CLOG_C_VERBOSE(&self->log, "cmd: %s", clvSerializeCmdToString(cmd))
#endif
    switch (data[0]) {
    case clvSerializeCmdRoomCreateResponse:
        return onRoomCreateResponse(self, &inStream);
    case clvSerializeCmdRoomJoinResponse:
        return onRoomJoinResponse(self, &inStream);
    case clvSerializeCmdRoomReJoinResponse:
        return onRoomReJoinResponse(self, &inStream);
    case clvSerializeCmdListRoomsResponse:
        return onListRoomsResponse(self, &inStream);
    case clvSerializeCmdLoginResponse:
        return onLoginResponse(self, &inStream);
    default:
        CLOG_C_ERROR(&self->log, "unknown message %02X", cmd)
        //return -1;
    }
    //return 0;
}

int clvClientReceiveAllInUdpBuffer(ClvClient* self)
{
    size_t count = 0;
    while (1) {
        uint8_t receiveBuf[DATAGRAM_TRANSPORT_MAX_SIZE];
        ssize_t octetCount
            = datagramTransportReceive(&self->transport, receiveBuf, DATAGRAM_TRANSPORT_MAX_SIZE);
        if (octetCount > 0) {
            clvClientFeed(self, receiveBuf, (size_t)octetCount);
            count++;
        } else if (octetCount < 0) {
            CLOG_C_NOTICE(&self->log, "datagramTransportReceive: %zd", octetCount)
            return (int)octetCount;
        } else {
            break;
        }
    }

    return (int)count;
}
