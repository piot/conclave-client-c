/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef CONCLAVE_CLIENT_NETWORK_REALIZE_H
#define CONCLAVE_CLIENT_NETWORK_REALIZE_H

#include <conclave-client/client.h>
#include <conclave-client/network_realizer.h>
#include <conclave-serialize/types.h>
#include <stddef.h>

struct ImprintAllocator;

typedef enum ClvClientRealizeState {
    ClvClientRealizeStateInit,
    ClvClientRealizeStateReInit,
    ClvClientRealizeStateCleared,
    ClvClientRealizeStateCreateRoom,
    ClvClientRealizeStateJoinRoom,
    ClvClientRealizeStateListRooms,
    ClvClientRealizeStateListRoomsDone,
} ClvClientRealizeState;

typedef struct ClvClientRealizeSettings {
    DatagramTransport transport;
    const char* username;
    struct ImprintAllocator* memory;
    Clog log;
} ClvClientRealizeSettings;

typedef struct ClvClientRealize {
    ClvClientRealizeState targetState;
    ClvClientRealizeState state;

    ClvClient client;
    ClvClientRealizeSettings settings;

    ClvSerializeRoomCreateOptions createRoomOptions;
    ClvSerializeRoomJoinOptions joinRoomOptions;
    ClvSerializeListRoomsOptions listRoomsOptions;

    bool isInRoom;
} ClvClientRealize;

void clvClientRealizeInit(ClvClientRealize* self, const ClvClientRealizeSettings* settings);
void clvClientRealizeReInit(ClvClientRealize* self, const ClvClientRealizeSettings* settings);
void clvClientRealizeDestroy(ClvClientRealize* self);
void clvClientRealizeReset(ClvClientRealize* self);
void clvClientRealizeQuitGame(ClvClientRealize* self);
void clvClientRealizeCreateRoom(ClvClientRealize* self, const ClvSerializeRoomCreateOptions* roomOptions);
void clvClientRealizeJoinRoom(ClvClientRealize* self, const ClvSerializeRoomJoinOptions* joinRoom);
void clvClientRealizeListRooms(ClvClientRealize* self, const ClvSerializeListRoomsOptions* listRooms);
void clvClientRealizeUpdate(ClvClientRealize* self, MonotonicTimeMs now);
int clvClientRealizeSendPacket(ClvClientRealize* self, int connectionId, const uint8_t* octets, size_t octetCount);
int clvClientRealizeReadPacket(ClvClientRealize* self, int* connectionId, uint8_t* octets, size_t octetCount);

#endif
