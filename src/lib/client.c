/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <conclave-client/client.h>
#include <conclave-client/incoming.h>
#include <conclave-client/outgoing.h>

void clvClientReInit(ClvClient* self, UdpTransportInOut* transport)
{
    self->transport = *transport;
    self->state = ClvClientStateConnected;
}

int clvClientInit(ClvClient* self, struct ImprintAllocator* memory, UdpTransportInOut* transport, Clog log)
{
    self->log = log;
    self->name = 0;
    self->memory = memory;
    self->state = ClvClientStateConnected;
    self->transport = *transport;
    discoidBufferInit(&self->inBuffer, memory, 32 * 1024);

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
