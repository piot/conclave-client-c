/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <conclave-client/client.h>
#include <conclave-client/incoming.h>
#include <conclave-client/outgoing.h>

void clvClientReInit(ClvClient* self, UdpTransportInOut* transport)
{
    self->localParticipantCount = 0;
    for (size_t i = 0; i < CONCLAVE_CLIENT_MAX_LOCAL_USERS_COUNT; ++i) {
        self->localParticipantLookup[i].participantId = 0;
        self->localParticipantLookup[i].localUserDeviceIndex = 0;
    }
    self->transport = *transport;
    self->state = ClvClientStateConnected;
}

int clvClientInit(ClvClient* self, struct ImprintAllocator* memory, struct ImprintAllocatorWithFree* blobAllocator,
                  UdpTransportInOut* transport)
{
    clogInitFromGlobal(&self->clog, "ClvClient");
    self->name = 0;
    self->memory = memory;
    self->state = ClvClientStateConnected;
    self->transport = *transport;
    self->blobStreamAllocator = blobAllocator;

    return 0;
}

void clvClientDestroy(ClvClient* self)
{
}

void clvClientDisconnect(ClvClient* self)
{
}

static int sendPackets(ClvClient* self, MonotonicTimeMs now)
{
    UdpTransportOut transportOut;
    transportOut.self = self->transport.self;
    transportOut.send = self->transport.send;

    int errorCode = clvClientOutgoing(self, now, &transportOut);
    if (errorCode < 0) {
        return errorCode;
    }

    return 0;
}

int clvClientUpdate(ClvClient* self, MonotonicTimeMs now)
{
    int errorCode;

    errorCode = clvClientReceiveAllInUdpBuffer(self);
    if (errorCode < 0) {
        return errorCode;
    }

    self->waitTime--;
    if (self->waitTime > 0) {
        return 0;
    }
    sendPackets(self, now);

    return errorCode;
}
