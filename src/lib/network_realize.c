/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <conclave-client/client.h>
#include <conclave-client/network_realizer.h>
#include <conclave-client/outgoing.h>
#include <imprint/allocator.h>

void clvClientRealizeInit(ClvClientRealize* self, const ClvClientRealizeSettings* settings)
{
    self->targetState = ClvClientRealizeStateInit;
    self->state = ClvClientRealizeStateInit;
    self->settings = *settings;
    self->settings.username = tc_str_dup(self->settings.username);
    self->isInRoom = false;
    clvClientInit(&self->client, settings->memory, &self->settings.transport, settings->log);
}

void clvClientRealizeReInit(ClvClientRealize* self, const ClvClientRealizeSettings* settings)
{
    self->targetState = ClvClientRealizeStateReInit;
    self->state = ClvClientRealizeStateReInit;
    self->settings = *settings;
    self->settings.username = tc_str_dup(self->settings.username);
    self->isInRoom = false;
    clvClientReInit(&self->client, &self->settings.transport);
}

void clvClientRealizeDestroy(ClvClientRealize* self)
{
    tc_free((void*)self->settings.username);
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

static void tryConnectAndLogin(ClvClientRealize* self)
{
    switch (self->client.state) {
    case ClvClientStateConnected:
        clvClientLogin(&self->client, self->settings.username);
        break;
    case ClvClientStateIdle:
        break;
    case ClvClientStateConnecting:
        break;
    case ClvClientStateLogin:
        break;
    case ClvClientStateLoggedIn:
        break;
    case ClvClientStateRoomCreate:
        break;
    case ClvClientStateRoomJoin:
        break;
    case ClvClientStateRoomReJoin:
        break;
    case ClvClientStatePlaying:
        break;
    case ClvClientStateListRooms:
        break;
    case ClvClientStateListRoomDone:
        break;
    }
}

static void tryCreateRoom(ClvClientRealize* self)
{
    tryConnectAndLogin(self);
    switch (self->client.state) {
    case ClvClientStateLoggedIn:
        clvClientRoomCreate(&self->client, &self->createRoomOptions);
        break;
    case ClvClientStatePlaying:
        self->state = ClvClientRealizeStateCreateRoom;
        self->isInRoom = true;
        break;
    case ClvClientStateIdle:
        break;
    case ClvClientStateConnecting:
        break;
    case ClvClientStateConnected:
        break;
    case ClvClientStateLogin:
        break;
    case ClvClientStateRoomCreate:
        break;
    case ClvClientStateRoomJoin:
        break;
    case ClvClientStateRoomReJoin:
        break;
    case ClvClientStateListRooms:
        break;
    case ClvClientStateListRoomDone:
        break;
    }
}

static void tryJoinRoom(ClvClientRealize* self)
{
    tryConnectAndLogin(self);
    switch (self->client.state) {
    case ClvClientStateLoggedIn:
    case ClvClientStateListRoomDone:
        clvClientRoomJoin(&self->client, &self->joinRoomOptions);
        break;
    case ClvClientStatePlaying:
        self->state = ClvClientRealizeStateJoinRoom;
        self->isInRoom = true;
        break;
    case ClvClientStateIdle:
        break;
    case ClvClientStateConnecting:
        break;
    case ClvClientStateConnected:
        break;
    case ClvClientStateLogin:
        break;
    case ClvClientStateRoomCreate:
        break;
    case ClvClientStateRoomJoin:
        break;
    case ClvClientStateRoomReJoin:
        break;
    case ClvClientStateListRooms:
        break;
    }
}

static void tryListRooms(ClvClientRealize* self)
{
    tryConnectAndLogin(self);
    switch (self->client.state) {
    case ClvClientStateLoggedIn:
        clvClientListRooms(&self->client, &self->listRoomsOptions);
        break;
    case ClvClientStateListRoomDone:
        self->state = ClvClientRealizeStateListRoomsDone;
        break;
    case ClvClientStateIdle:
        break;
    case ClvClientStateConnecting:
        break;
    case ClvClientStateConnected:
        break;
    case ClvClientStateLogin:
        break;
    case ClvClientStateRoomCreate:
        break;
    case ClvClientStateRoomJoin:
        break;
    case ClvClientStateRoomReJoin:
        break;
    case ClvClientStateListRooms:
        break;
    case ClvClientStatePlaying:
        break;
    }
}

void clvClientRealizeUpdate(ClvClientRealize* self, MonotonicTimeMs now)
{
    if (self->state != ClvClientRealizeStateCleared
        && self->targetState != ClvClientRealizeStateInit) {
        clvClientUpdate(&self->client, now);
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
    }
}

