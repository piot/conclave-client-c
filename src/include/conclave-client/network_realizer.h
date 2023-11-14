/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef CONCLAVE_CLIENT_NETWORK_REALIZE_H
#define CONCLAVE_CLIENT_NETWORK_REALIZE_H

#include <conclave-client/client.h>
#include <conclave-client/network_realizer.h>
#include <conclave-serialize/types.h>
#include <stdbool.h>

struct ImprintAllocator;

typedef enum ClvClientRealizeState {
    ClvClientRealizeStateInit,
    ClvClientRealizeStateReInit,
    ClvClientRealizeStateCleared,
    ClvClientRealizeStateLoggedIn,
    ClvClientRealizeStateCreateRoom,
    ClvClientRealizeStateJoinRoom,
    ClvClientRealizeStateListRooms,
    ClvClientRealizeStateListRoomsDone,
} ClvClientRealizeState;

typedef struct ClvClientRealizeSettings {
    DatagramTransport transport;
    GuiseSerializeUserSessionId guiseUserSessionId;
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

    Clog log;
    char subLog[32];

    bool isInRoom;
} ClvClientRealize;

int clvClientRealizeInit(ClvClientRealize* self, const ClvClientRealizeSettings* settings);
void clvClientRealizeReInit(ClvClientRealize* self, const ClvClientRealizeSettings* settings);
void clvClientRealizeDestroy(ClvClientRealize* self);
void clvClientRealizeReset(ClvClientRealize* self);
void clvClientRealizeQuitGame(ClvClientRealize* self);
void clvClientRealizeCreateRoom(
    ClvClientRealize* self, const ClvSerializeRoomCreateOptions* roomOptions);
void clvClientRealizeJoinRoom(ClvClientRealize* self, const ClvSerializeRoomJoinOptions* joinRoom);
void clvClientRealizeListRooms(
    ClvClientRealize* self, const ClvSerializeListRoomsOptions* listRooms);
int clvClientRealizeUpdate(ClvClientRealize* self, MonotonicTimeMs now);

#endif
