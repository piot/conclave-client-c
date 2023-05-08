/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <conclave-client/client.h>
#include <conclave-client/incoming.h>
#include <conclave-serialize/client_in.h>
#include <conclave-serialize/debug.h>
#include <conclave-serialize/serialize.h>
#include <flood/in_stream.h>
#include <imprint/allocator.h>

static int onRoomCreateResponse(ClvClient* self, FldInStream* inStream)
{
    uint32_t roomId;
    clvSerializeReadRoomId(inStream, &roomId);

    uint8_t roomConnectionIndex;
    int readErr = clvSerializeReadRoomConnectionIndex(inStream, &roomConnectionIndex);
    if (readErr < 0) {
        return readErr;
    }

    self->state = ClvClientStatePlaying;
    self->waitTime = 0;
    self->mainRoomId = roomId;
    self->roomConnectionIndex = roomConnectionIndex;

    CLOG_C_INFO(&self->log, "room create response. room id: %d connectionIndex: %d", roomId, roomConnectionIndex)

    return 0;
}

static int onRoomJoinResponse(ClvClient* self, FldInStream* inStream)
{
    uint32_t roomId;
    clvSerializeReadRoomId(inStream, &roomId);

    uint8_t roomConnectionIndex;
    int err = clvSerializeReadRoomConnectionIndex(inStream, &roomConnectionIndex);
    if (err < 0) {
        return err;
    }

    self->mainRoomId = roomId;
    self->roomConnectionIndex = roomConnectionIndex;

    CLOG_C_INFO(&self->log, "room join response. room id: %d. connection index %d", roomId, self->roomConnectionIndex)

    self->state = ClvClientStatePlaying;

    self->reJoinRoomOptions.roomId = roomId;
    self->reJoinRoomOptions.roomConnectionIndex = self->roomConnectionIndex;

    return 0;
}

static int onListRoomsResponse(ClvClient* self, FldInStream* inStream)
{
    clvSerializeClientInListRoomsResponse(inStream, &self->listRoomsResponseOptions);

    CLOG_C_INFO(&self->log, "got list of rooms back %zu", self->listRoomsResponseOptions.roomInfoCount);
    for (size_t i = 0; i < self->listRoomsResponseOptions.roomInfoCount; ++i) {
        const ClvSerializeRoomInfo* roomInfo = &self->listRoomsResponseOptions.roomInfos[i];
        CLOG_C_INFO(&self->log, " %zu: %d '%s' '%s'", i, roomInfo->roomId, roomInfo->roomName, roomInfo->hostUserName)
    }

    self->state = ClvClientStateListRoomDone;
    self->waitTime = 160;

    return 0;
}

static int onRoomReJoinResponse(ClvClient* self, FldInStream* inStream)
{
    uint32_t roomId;
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
    uint32_t sessionId;
    fldInStreamReadUInt32(inStream, &sessionId);

    CLOG_C_INFO(&self->log, "Logged in as session %d", sessionId);

    self->mainUserSessionId = sessionId;
    self->state = ClvClientStateLoggedIn;

    return 0;
}

static int onIncomingPacket(ClvClient* self, FldInStream* inStream)
{
    uint8_t fromConnectionIndexInRoom;
    fldInStreamReadUInt8(inStream, &fromConnectionIndexInRoom);
    uint16_t octetCountInPacket;
    fldInStreamReadUInt16(inStream, &octetCountInPacket);

    if (discoidBufferWriteAvailable(&self->inBuffer) < octetCountInPacket + 1 + 2) {
        CLOG_C_NOTICE(&self->log, "dropping packets since in buffer is full")
        return 0;
    }

    discoidBufferWrite(&self->inBuffer, &fromConnectionIndexInRoom, 1);
    discoidBufferWrite(&self->inBuffer, (uint8_t*) &octetCountInPacket, 2);
    discoidBufferWrite(&self->inBuffer, inStream->p, octetCountInPacket);

    inStream->p += octetCountInPacket;
    inStream->pos += octetCountInPacket;

    return 0;
}

static int clvClientFeed(ClvClient* self, const uint8_t* data, size_t len)
{
    FldInStream inStream;
    fldInStreamInit(&inStream, data, len);

    uint8_t cmd;
    fldInStreamReadUInt8(&inStream, &cmd);
    CLOG_C_INFO(&self->log, "cmd: %s", clvSerializeCmdToString(cmd));
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
        case clvSerializeCmdPacketToClient:
            return onIncomingPacket(self, &inStream);
            return -4;
        default:
            CLOG_C_ERROR(&self->log, "unknown message %02X", cmd)
            return -1;
    }
    return 0;
}

int clvClientReceiveAllInUdpBuffer(ClvClient* self)
{
#define UDP_MAX_RECEIVE_BUF_SIZE (64000)
    uint8_t receiveBuf[UDP_MAX_RECEIVE_BUF_SIZE];
    size_t count = 0;
    while (1) {
        int octetCount = udpTransportReceive(&self->transport, receiveBuf, UDP_MAX_RECEIVE_BUF_SIZE);
        if (octetCount > 0) {
            clvClientFeed(self, receiveBuf, octetCount);
            count++;
        } else if (octetCount < 0) {
            printf("error: %d\n", octetCount);
            return octetCount;
        } else {
            break;
        }
    }

    return count;
}
