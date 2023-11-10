/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef CONCLAVE_CLIENT_OUTGOING_API_H
#define CONCLAVE_CLIENT_OUTGOING_API_H

#include <conclave-serialize/types.h>
#include <stddef.h>

struct ClvClient;

int clvClientLogin(struct ClvClient* self, const char* name);
int clvClientRoomCreate(struct ClvClient* self, const ClvSerializeRoomCreateOptions* rooms);
int clvClientRoomJoin(struct ClvClient* self, const ClvSerializeRoomJoinOptions* name);
int clvClientReJoin(struct ClvClient* self);
int clvClientListRooms(struct ClvClient* self, const ClvSerializeListRoomsOptions* options);

#endif
