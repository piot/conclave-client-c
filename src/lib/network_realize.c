/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <conclave-client/client.h>
#include <conclave-client/network_realizer.h>
#include <conclave-client/outgoing.h>
#include <conclave-client/realize_debug.h>
#include <imprint/allocator.h>
#include <stdbool.h>

int clvClientRealizeInit(ClvClientRealize* self, const ClvClientRealizeSettings* settings)
{
    self->targetState = ClvClientRealizeStateLoggedIn;
    self->state = ClvClientRealizeStateInit;
    self->log = settings->log;
    self->settings = *settings;
    self->isInRoom = false;

    Clog clvClientLog;
    clvClientLog.config = settings->log.config;
    tc_snprintf(self->subLog, 32, "%s/client", self->log.constantPrefix);
    clvClientLog.constantPrefix = self->subLog;

    return clvClientInit(
        &self->client, &self->settings.transport, settings->guiseUserSessionId, clvClientLog);
}

void clvClientRealizeReInit(ClvClientRealize* self, const ClvClientRealizeSettings* settings)
{
    self->targetState = ClvClientRealizeStateCleared;
    self->state = ClvClientRealizeStateReInit;
    self->log = settings->log;
    self->settings = *settings;
    self->isInRoom = false;
    clvClientReInit(&self->client, &self->settings.transport);
}

void clvClientRealizeDestroy(ClvClientRealize* self)
{
    clvClientDestroy(&self->client);
}

void clvClientRealizeReset(ClvClientRealize* self)
{
    // clvClientReset(&self->client);
    self->state = ClvClientRealizeStateCleared;
}

void clvClientRealizeQuitGame(ClvClientRealize* self)
{
    clvClientDisconnect(&self->client);
    self->targetState = ClvClientRealizeStateCleared;
}

void clvClientRealizeCreateRoom(
    ClvClientRealize* self, const ClvSerializeRoomCreateOptions* createRoom)
{
    self->createRoomOptions = *createRoom;
    self->createRoomOptions.name = tc_str_dup(createRoom->name);

    self->targetState = ClvClientRealizeStateCreateRoom;
}

void clvClientRealizeJoinRoom(ClvClientRealize* self, const ClvSerializeRoomJoinOptions* joinRoom)
{
    self->joinRoomOptions = *joinRoom;
    self->joinRoomOptions.roomIdToJoin = joinRoom->roomIdToJoin;
    self->targetState = ClvClientRealizeStateJoinRoom;
}

void clvClientRealizeListRooms(
    ClvClientRealize* self, const ClvSerializeListRoomsOptions* listRooms)
{
    self->listRoomsOptions = *listRooms;
    self->targetState = ClvClientRealizeStateListRooms;
}

static void sendLoginIfNeeded(ClvClientRealize* self)
{
    switch (self->client.state) {
    case ClvClientStateLogin:
        clvClientLogin(&self->client, self->settings.guiseUserSessionId);
        break;
    default:
        break;
    }
}

static void tryCreateRoom(ClvClientRealize* self)
{
    sendLoginIfNeeded(self);
    switch (self->client.state) {
    case ClvClientStateLoggedIn:
        clvClientRoomCreate(&self->client, &self->createRoomOptions);
        break;
    case ClvClientStatePlaying:
        CLOG_C_INFO(&self->log, "room created")
        self->state = ClvClientRealizeStateCreateRoom;
        self->targetState = ClvClientRealizeStateLoggedIn;
        self->isInRoom = true;
        break;
    default:
        break;
    }
}

static void tryJoinRoom(ClvClientRealize* self)
{
    sendLoginIfNeeded(self);
    switch (self->client.state) {
    case ClvClientStateLoggedIn:
    case ClvClientStateListRoomDone:
        clvClientRoomJoin(&self->client, &self->joinRoomOptions);
        break;
    case ClvClientStatePlaying:
        self->state = ClvClientRealizeStateJoinRoom;
        self->isInRoom = true;
        break;
    default:
        break;
    }
}

static void tryListRooms(ClvClientRealize* self)
{
    sendLoginIfNeeded(self);
    switch (self->client.state) {
    case ClvClientStateLoggedIn:
        clvClientListRooms(&self->client, &self->listRoomsOptions);
        break;
    case ClvClientStateListRoomDone:
        self->state = ClvClientRealizeStateListRoomsDone;
        break;
    default:
        break;
    }
}

int clvClientRealizePing(ClvClientRealize* self, uint64_t knowledge)
{
    return clvClientPing(&self->client, knowledge);
}

int clvClientRealizeUpdate(ClvClientRealize* self, MonotonicTimeMs now)
{
    int result = 0;

#if defined CLOG_LOG_ENABLED && false
    clvClientRealizeDebugOutput(self);
#endif

    if (self->state != ClvClientRealizeStateCleared
        && self->targetState != ClvClientRealizeStateInit) {
        result = clvClientUpdate(&self->client, now);
    }

    if (self->targetState != ClvClientRealizeStateInit
        && self->targetState != ClvClientRealizeStateCleared
        && self->client.state == ClvClientStateIdle) {
        self->client.state = ClvClientStateLogin;
    }

    switch (self->targetState) {
    case ClvClientRealizeStateInit:
    case ClvClientRealizeStateReInit:
    case ClvClientRealizeStateListRoomsDone:
        break;
    case ClvClientRealizeStateCreateRoom:
        tryCreateRoom(self);
        break;
    case ClvClientRealizeStateJoinRoom:
        tryJoinRoom(self);
        break;
    case ClvClientRealizeStateListRooms:
        tryListRooms(self);
        break;
    case ClvClientRealizeStateCleared:
        if (self->state != ClvClientRealizeStateCleared) {
            self->state = self->targetState;
        }
        break;
    case ClvClientRealizeStateLoggedIn:
        //tryConnectAndLogin(self);
        break;
    }

    return result;
}
