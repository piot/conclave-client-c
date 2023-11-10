/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef CONCLAVE_CLIENT_OUTGOING_H
#define CONCLAVE_CLIENT_OUTGOING_H

struct ClvClient;
struct DatagramTransportOut;

#include <monotonic-time/monotonic_time.h>

int clvClientOutgoing(struct ClvClient* self, MonotonicTimeMs now, struct DatagramTransportOut* transportOut);


#endif
