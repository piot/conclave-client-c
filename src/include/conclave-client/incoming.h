/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef CONCLAVE_CLIENT_INCOMING_H
#define CONCLAVE_CLIENT_INCOMING_H

struct ClvClient;

int clvClientReceiveAllInUdpBuffer(struct ClvClient* self);

#endif
