/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef CONCLAVE_CLIENT_CLIENT_H
#define CONCLAVE_CLIENT_CLIENT_H

#include <clog/clog.h>
#include <conclave-serialize/client_out.h>
#include <datagram-reassembly/reassembly.h>
#include <datagram-transport/transport.h>
#include <monotonic-time/monotonic_time.h>
#include <stdint.h>
#include <time-tick/time_tick.h>

struct FldOutStream;
struct ImprintAllocatorWithFree;

typedef enum ClvClientState {
    ClvClientStateIdle,
    ClvClientStateLogIn,
    ClvClientStateLoggedIn,
    ClvClientStateRoomCreate,
    ClvClientStateRoomList,
    ClvClientStateRoomJoin,
    ClvClientStateRoomReJoin,
} ClvClientState;

#define CONCLAVE_CLIENT_MAX_LOCAL_USERS_COUNT (8)

struct ImprintAllocator;

typedef struct ClvClient {
    ClvClientState state;
    ClvSerializeRoomCreateOptions createRoomOptions;
    uint8_t roomCreateVersion;
    ClvSerializeRoomJoinOptions joinRoomOptions;
    ClvSerializeRoomReJoinOptions reJoinRoomOptions;
    ClvSerializeListRoomsOptions listRoomsOptions;
    uint8_t listRoomsOptionsVersion;
    ClvSerializeListRoomsResponseOptions listRoomsResponseOptions;
    ClvSerializePingResponseOptions pingResponseOptions;
    uint8_t pingResponseOptionsVersion;
    uint8_t localPlayerIndex;
    ClvSerializeUserSessionId mainUserSessionId;
    GuiseSerializeUserSessionId guiseUserSessionId;
    ClvSerializeRoomId mainRoomId;
    ClvSerializeRoomConnectionIndex roomConnectionIndex;
    ClvSerializeClientNonce nonce;
    DatagramTransport transport;
    DatagramReassembly reassembly;
    struct ImprintAllocatorWithFree* allocatorWithFree;
    TimeTick timeTick;
    char timeTickLogName[32];
    size_t frame;
    Clog log;
} ClvClient;

int clvClientInit(ClvClient* self, const DatagramTransport* transport,
    const GuiseSerializeUserSessionId guiseUserSessionId, MonotonicTimeMs now,
    struct ImprintAllocatorWithFree* allocator, const Clog log);
void clvClientReset(ClvClient* self);
void clvClientReInit(ClvClient* self, DatagramTransport* transport);
void clvClientDestroy(ClvClient* self);
void clvClientDisconnect(ClvClient* self);
int clvClientUpdate(ClvClient* self, MonotonicTimeMs now);
int clvClientFindParticipantId(
    const ClvClient* self, uint8_t localUserDeviceIndex, uint8_t* participantId);
int clvClientReJoin(ClvClient* self);
int clvClientPing(ClvClient* self, uint64_t knowledge, bool hasConnectionToOwner);
int clvClientCreateRoom(ClvClient* self, const ClvSerializeRoomCreateOptions* roomOptions);
void clvClientJoinRoom(ClvClient* self, const ClvSerializeRoomJoinOptions* joinRoom);
void clvClientListRooms(ClvClient* self, const ClvSerializeListRoomsOptions* listRooms);
int clvClientLogin(ClvClient* self, GuiseSerializeUserSessionId guiseUserSessionId);

#endif
