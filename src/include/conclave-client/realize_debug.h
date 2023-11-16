/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef CONCLAVE_CLIENT_REALIZE_DEBUG_H
#define CONCLAVE_CLIENT_REALIZE_DEBUG_H

struct ClvClientRealize;

void clvClientRealizeDebugOutput(const struct ClvClientRealize* self);
const char* clvClientRealizeStateToString(ClvClientRealizeState state);

#endif
