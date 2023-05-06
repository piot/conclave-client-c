/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef CONCLAVE_CLIENT_INCOMING_API_H
#define CONCLAVE_CLIENT_INCOMING_API_H

#include <stdint.h>

struct ClvClient;

int clvClientReadPacket(struct ClvClient* self, const uint8_t** target, int* outStepId);

#endif
