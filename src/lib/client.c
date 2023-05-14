/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <conclave-client/client.h>
#include <conclave-client/incoming.h>
#include <conclave-client/incoming_api.h>
#include <conclave-client/outgoing.h>
#include <secure-random/secure_random.h>

void clvClientReInit(ClvClient* self, DatagramTransport* transport)
{
    self->transport = *transport;
    self->state = ClvClientStateConnected;
}
static int multiTransportSend(void* _self, int connectionIndex, const uint8_t* data, size_t size)
{
    ClvClient* self = (ClvClient*) _self;

    CLOG_C_DEBUG(&self->log, "sending to relay: connection:%d octetCount:%d", connectionIndex, size)

    return clvClientOutAddPacket(self, connectionIndex, data, size);
}

static int multiTransportReceive(void* _self, int* receivedFromConnectionIndex, uint8_t* data, size_t size)
{
    ClvClient* self = (ClvClient*) _self;

    int octetCount = clvClientInReadPacket(self, receivedFromConnectionIndex, data, size);
    if (octetCount < 0) {
        CLOG_C_SOFT_ERROR(&self->log, "could not read in packet from clv")
    }
    if (octetCount > 0) {
        CLOG_C_DEBUG(&self->log, "got packet from relay: connection:%d octetCount:%d", *receivedFromConnectionIndex,
                     octetCount)
    }

    return octetCount;
}

int clvClientInit(ClvClient* self, struct ImprintAllocator* memory, DatagramTransport* transport, Clog log)
{
    self->log = log;
    self->name = 0;
    self->memory = memory;
    self->state = ClvClientStateConnected;
    self->transport = *transport;
    self->nonce = secureRandomUInt64();
    discoidBufferInit(&self->inBuffer, memory, 32 * 1024);

    self->multiTransport.self = self;
    self->multiTransport.sendTo = multiTransportSend;
    self->multiTransport.receiveFrom = multiTransportReceive;

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
