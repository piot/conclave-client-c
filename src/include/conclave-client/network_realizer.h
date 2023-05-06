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
    ClvClientRealizeStateJoinRoom
} ClvClientRealizeState;

typedef struct ClvClientRealizeSettings {
    UdpTransportInOut transport;
    const char* username;
    struct ImprintAllocator* memory;
} ClvClientRealizeSettings;

typedef struct ClvClientRealize {
    ClvClientRealizeState targetState;
    ClvClientRealizeState state;
    ClvClient client;
    ClvClientRealizeSettings settings;

    ClvSerializeRoomCreateOptions createRoomOptions;

    ClvSerializeRoomJoinOptions joinRoomOptions;
} ClvClientRealize;

void clvClientRealizeInit(ClvClientRealize* self, const ClvClientRealizeSettings* settings);
void clvClientRealizeReInit(ClvClientRealize* self, const ClvClientRealizeSettings* settings);
void clvClientRealizeDestroy(ClvClientRealize* self);
void clvClientRealizeReset(ClvClientRealize* self);
void clvClientRealizeQuitGame(ClvClientRealize* self);
void clvClientRealizeCreateRoom(ClvClientRealize* self, const ClvSerializeRoomCreateOptions* roomOptions);
void clvClientRealizeJoinRoom(ClvClientRealize* self, const ClvSerializeRoomJoinOptions* joinRoom);
void clvClientRealizeUpdate(ClvClientRealize* self, MonotonicTimeMs now);
int clvClientRealizeSendPacket(ClvClientRealize* self, int connectionId, const uint8_t* octets, size_t octetCount);

#endif
