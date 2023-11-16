/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef CONCLAVE_CLIENT_DEBUG_H
#define CONCLAVE_CLIENT_DEBUG_H

struct ClvClient;

void clvClientDebugOutput(const struct ClvClient* self);
const char* clvClientStateToString(ClvClientState state);

#endif
