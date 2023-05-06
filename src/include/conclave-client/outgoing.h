/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef CONCLAVE_CLIENT_OUTGOING_H
#define CONCLAVE_CLIENT_OUTGOING_H

struct ClvClient;
struct UdpTransportOut;

#include <monotonic-time/monotonic_time.h>

int clvClientOutgoing(struct ClvClient* self, MonotonicTimeMs now, struct UdpTransportOut* transportOut);
int clvClientOutAddPacket(struct ClvClient* self, int toMemberId, const uint8_t* octets, size_t octetCount);

#endif
