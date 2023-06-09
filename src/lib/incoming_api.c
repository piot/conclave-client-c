/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <conclave-client/client.h>
#include <conclave-client/incoming_api.h>

int clvClientInReadPacket(struct ClvClient* self, int* connectionId, uint8_t* octets, size_t octetCount)
{
    int count = discoidBufferReadAvailable(&self->inBuffer);
    if (count < 1) {
        return 0;
    }

    uint8_t fromConnectionId;
    discoidBufferRead(&self->inBuffer, &fromConnectionId, 1);

    uint16_t followingOctets;
    discoidBufferRead(&self->inBuffer, (uint8_t*) &followingOctets, 2);

    *connectionId = fromConnectionId;

    if (octetCount < followingOctets) {
        CLOG_C_SOFT_ERROR(&self->log, "can not read incoming packet from circular buffer")
        discoidBufferSkip(&self->inBuffer, followingOctets);
        return -2;
    }

    discoidBufferRead(&self->inBuffer, octets, followingOctets);

    return followingOctets;
}
