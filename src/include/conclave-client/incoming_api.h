/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef CONCLAVE_CLIENT_INCOMING_API_H
#define CONCLAVE_CLIENT_INCOMING_API_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct ClvClient;

int clvClientInReadPacket(struct ClvClient* self, int* connectionId, uint8_t* octets, size_t octetCount);

#endif
