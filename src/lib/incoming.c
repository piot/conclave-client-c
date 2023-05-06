/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <conclave-client/client.h>
#include <conclave-client/incoming.h>
#include <conclave-serialize/debug.h>
#include <conclave-serialize/serialize.h>
#include <flood/in_stream.h>
#include <imprint/allocator.h>

static int readParticipantConnectionIdAndParticipants(ClvClient* self, FldInStream* inStream)
{
    fldInStreamReadUInt8(inStream, &self->participantsConnectionIndex);

    uint8_t participantCount;
    fldInStreamReadUInt8(inStream, &participantCount);
    if (participantCount > CONCLAVE_CLIENT_MAX_LOCAL_USERS_COUNT) {
        CLOG_ERROR("we can not have more than %d local users (%d)", CONCLAVE_CLIENT_MAX_LOCAL_USERS_COUNT,
                   participantCount)
        return -1;
    }

    self->localParticipantCount = participantCount;

    for (size_t i = 0; i < participantCount; ++i) {
        uint8_t localIndex;
        fldInStreamReadUInt8(inStream, &localIndex);
        uint8_t participantId;
        fldInStreamReadUInt8(inStream, &participantId);
        CLOG_INFO("** -> I am participant id: %d (localIndex:%d)", participantId, localIndex)
        self->localParticipantLookup[i].localUserDeviceIndex = localIndex;
        self->localParticipantLookup[i].participantId = participantId;
    }

    return participantCount;
}

static int onRoomCreateResponse(ClvClient* self, FldInStream* inStream)
{
    uint32_t roomId;
    clvSerializeReadRoomId(inStream, &roomId);

    int readErr = readParticipantConnectionIdAndParticipants(self, inStream);
    if (readErr < 0) {
        return readErr;
    }

    CLOG_INFO("room create response. room id: %d channel %d", roomId, self->roomCreateChannelId)

    self->state = ClvClientStatePlaying;
    self->waitTime = 0;
    self->mainRoomId = roomId;

    return 0;
}

static int onRoomJoinResponse(ClvClient* self, FldInStream* inStream)
{
    uint32_t roomId;
    clvSerializeReadRoomId(inStream, &roomId);

    int participantCount = readParticipantConnectionIdAndParticipants(self, inStream);
    if (participantCount < 0) {
        return participantCount;
    }

    CLOG_INFO("room join response. room id: %d. connection index %d participants: %d", roomId,
              self->participantsConnectionIndex, participantCount)

    self->state = ClvClientStatePlaying;
    self->mainRoomId = roomId;

    self->reJoinRoomOptions.roomId = roomId;
    self->reJoinRoomOptions.roomConnectionIndex = self->participantsConnectionIndex;
    clvClientReJoin(self);

    /*
    self->state = clvClientReJoin(self);
    self->waitTime = 0;


    self->nextStepIdToSendToServer = self->joinedGameState.stepId;
    */

    return 0;
}

static int onRoomReJoinResponse(ClvClient* self, FldInStream* inStream)
{
    CLOG_VERBOSE("rejoin answer: stateId: %04X octetCount:%zu channel:%02X", stateId, octetCount, channelId)

    self->state = ClvClientStatePlaying;
    //    self->hostState = ClvClientHostStateClient;
    self->waitTime = 0;

    return 0;
}

static int onLoginResponse(ClvClient* self, FldInStream* inStream)
{
    uint32_t sessionId;
    fldInStreamReadUInt32(inStream, &sessionId);

    CLOG_INFO("Logged in as session %d", sessionId);

    self->mainUserSessionId = sessionId;
    self->state = ClvClientStateLoggedIn;

    return 0;
}

static int clvClientFeed(ClvClient* self, const uint8_t* data, size_t len)
{
    FldInStream inStream;
    fldInStreamInit(&inStream, data, len);

    uint8_t cmd;
    fldInStreamReadUInt8(&inStream, &cmd);
    CLOG_INFO("clvClient: cmd: %s", clvSerializeCmdToString(cmd));
    switch (data[0]) {
        case clvSerializeCmdRoomCreateResponse:
            return onRoomCreateResponse(self, &inStream);
        case clvSerializeCmdRoomJoinResponse:
            return onRoomJoinResponse(self, &inStream);
        case clvSerializeCmdRoomReJoinResponse:
            return onRoomReJoinResponse(self, &inStream);
        case clvSerializeCmdLoginResponse:
            return onLoginResponse(self, &inStream);
        default:
            CLOG_ERROR("unknown message %02X", cmd)
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
