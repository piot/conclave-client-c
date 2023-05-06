/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef CONCLAVE_CLIENT_OUTGOING_API_H
#define CONCLAVE_CLIENT_OUTGOING_API_H

#include <conclave-serialize/types.h>
#include <stddef.h>

struct ClvClient;

int clvClientLogin(struct ClvClient* self, const char* name);
int clvClientRoomCreate(struct ClvClient* self, const ClvSerializeRoomCreateOptions* rooms);
int clvClientJoinGame(struct ClvClient* self);
int clvClientRoomJoin(struct ClvClient* self, const ClvSerializeRoomJoinOptions* name);

#endif
