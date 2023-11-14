/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <conclave-client/client.h>
#include <conclave-client/incoming.h>
#include <conclave-client/outgoing.h>
#include <secure-random/secure_random.h>

void clvClientReInit(ClvClient* self, DatagramTransport* transport)
{
    self->transport = *transport;
    self->state = ClvClientStateIdle;
    self->waitTime = 0;
}

int clvClientInit(ClvClient* self,
    const DatagramTransport* transport, const GuiseSerializeUserSessionId guiseUserSessionId,
    Clog log)
{
    self->log = log;
    self->state = ClvClientStateIdle;
    self->transport = *transport;
    self->guiseUserSessionId = guiseUserSessionId;
    self->nonce = secureRandomUInt64();
    self->waitTime = 0;

    return 0;
}

void clvClientDestroy(ClvClient* self)
{
    (void)self; // TODO: destroy ClvClient
}

void clvClientDisconnect(ClvClient* self)
{
    (void)self; // TODO: disconnect ClvClient
}

static int sendPackets(ClvClient* self, MonotonicTimeMs now)
{
    DatagramTransportOut transportOut;
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
    int errorCode = clvClientReceiveAllInUdpBuffer(self);
    if (errorCode < 0) {
        return (int)errorCode;
    }

    self->waitTime--;
    if (self->waitTime > 0) {
        return 0;
    }

    errorCode = sendPackets(self, now);

    return errorCode;
}
