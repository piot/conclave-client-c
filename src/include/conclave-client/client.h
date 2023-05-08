/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef CONCLAVE_CLIENT_CLIENT_H
#define CONCLAVE_CLIENT_CLIENT_H

#include <clog/clog.h>
#include <conclave-client/outgoing_api.h>
#include <conclave-serialize/client_out.h>
#include <discoid/circular_buffer.h>
#include <monotonic-time/monotonic_time.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <udp-transport/udp_transport.h>

struct ImprintAllocator;

struct FldOutStream;

typedef enum ClvClientState {
    ClvClientStateIdle,
    ClvClientStateConnecting,
    ClvClientStateConnected,
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
    const char* name;

    ClvSerializeRoomCreateOptions createRoomOptions;
    ClvSerializeRoomJoinOptions joinRoomOptions;
    ClvSerializeRoomReJoinOptions reJoinRoomOptions;
    ClvSerializeListRoomsOptions listRoomsOptions;
    ClvSerializeListRoomsResponseOptions listRoomsResponseOptions;

    int waitTime;

    uint8_t localPlayerIndex;
    ClvClientState state;
    ClvSerializeUserSessionId mainUserSessionId;
    ClvSerializeRoomId mainRoomId;
    ClvSerializeRoomConnectionIndex roomConnectionIndex;

    UdpTransportInOut transport;

    size_t frame;

    struct ImprintAllocator* memory;
    DiscoidBuffer inBuffer;
    Clog log;
} ClvClient;

int clvClientInit(ClvClient* self, struct ImprintAllocator* memory, UdpTransportInOut* transport, Clog log);
void clvClientReset(ClvClient* self);
void clvClientReInit(ClvClient* self, UdpTransportInOut* transport);
void clvClientDestroy(ClvClient* self);
void clvClientDisconnect(ClvClient* self);
int clvClientUpdate(ClvClient* self, MonotonicTimeMs now);
int clvClientFindParticipantId(const ClvClient* self, uint8_t localUserDeviceIndex, uint8_t* participantId);
int clvClientReJoin(ClvClient* self);

#endif
