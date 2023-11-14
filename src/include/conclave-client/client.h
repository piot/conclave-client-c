/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef CONCLAVE_CLIENT_CLIENT_H
#define CONCLAVE_CLIENT_CLIENT_H

#include <clog/clog.h>
#include <conclave-client/outgoing_api.h>
#include <conclave-serialize/client_out.h>
#include <datagram-transport/transport.h>
#include <monotonic-time/monotonic_time.h>
#include <stdint.h>

struct FldOutStream;

typedef enum ClvClientState {
    ClvClientStateIdle,
    ClvClientStateLogin,
    ClvClientStateLoggedIn,
    ClvClientStateRoomCreate,
    ClvClientStateRoomJoin,
    ClvClientStateRoomReJoin,
    ClvClientStateListRooms,
    ClvClientStateListRoomDone,
    ClvClientStatePlaying,
} ClvClientState;

#define CONCLAVE_CLIENT_MAX_LOCAL_USERS_COUNT (8)

struct ImprintAllocator;

typedef struct ClvClient {
    ClvSerializeRoomCreateOptions createRoomOptions;
    ClvSerializeRoomJoinOptions joinRoomOptions;
    ClvSerializeRoomReJoinOptions reJoinRoomOptions;
    ClvSerializeListRoomsOptions listRoomsOptions;
    ClvSerializeListRoomsResponseOptions listRoomsResponseOptions;

    int waitTime;

    uint8_t localPlayerIndex;
    ClvClientState state;
    ClvSerializeUserSessionId mainUserSessionId;
    GuiseSerializeUserSessionId guiseUserSessionId;
    ClvSerializeRoomId mainRoomId;
    ClvSerializeRoomConnectionIndex roomConnectionIndex;
    ClvSerializeClientNonce nonce;
    DatagramTransport transport;

    size_t frame;
    Clog log;
} ClvClient;

int clvClientInit(ClvClient* self,


const DatagramTransport* transport,
    const GuiseSerializeUserSessionId guiseUserSessionId, const Clog log);
void clvClientReset(ClvClient* self);
void clvClientReInit(ClvClient* self, DatagramTransport* transport);
void clvClientDestroy(ClvClient* self);
void clvClientDisconnect(ClvClient* self);
int clvClientUpdate(ClvClient* self, MonotonicTimeMs now);
int clvClientFindParticipantId(
    const ClvClient* self, uint8_t localUserDeviceIndex, uint8_t* participantId);
int clvClientReJoin(ClvClient* self);

#endif
