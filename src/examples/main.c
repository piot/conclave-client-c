/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include <clog/console.h>
#include <conclave-client/client.h>
#include <conclave-client/network_realizer.h>
#include <conclave-serialize/debug.h>
#include <flood/out_stream.h>
#include <imprint/default_setup.h>
#include <stdio.h>
#include <udp-client/udp_client.h>
#include <unistd.h>

clog_config g_clog;

static int clientReceive(void* _self, uint8_t* data, size_t size)
{
    UdpClientSocket* self = _self;

    return udpClientReceive(self, data, size);
}

static int clientSend(void* _self, const uint8_t* data, size_t size)
{
    UdpClientSocket* self = _self;

    return udpClientSend(self, data, size);
}

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    g_clog.log = clog_console;
    g_clog.level = CLOG_TYPE_VERBOSE;

    CLOG_VERBOSE("example start")
    CLOG_VERBOSE("initialized")

    FldOutStream outStream;

    uint8_t buf[1024];
    fldOutStreamInit(&outStream, buf, 1024);

    ClvClientRealize clientRealize;
    ClvClientRealizeSettings settings;

    ImprintDefaultSetup memory;

    UdpTransportInOut transportInOut;

    imprintDefaultSetupInit(&memory, 16 * 1024 * 1024);

    int startupErr = udpClientStartup();
    if (startupErr < 0) {
        return startupErr;
    }

    UdpClientSocket udpClientSocket;
    udpClientInit(&udpClientSocket, "127.0.0.1", 27003);

    transportInOut.self = &udpClientSocket;
    transportInOut.receive = clientReceive;
    transportInOut.send = clientSend;

    settings.memory = &memory.tagAllocator.info;
    settings.transport = transportInOut;
    settings.username = "Piot";

    clvClientRealizeInit(&clientRealize, &settings);
    clvClientRealizeReInit(&clientRealize, &settings);

    if (false) {
        ClvSerializeRoomCreateOptions createRoomOptions;
        createRoomOptions.flags = 0;
        createRoomOptions.maxNumberOfPlayers = 4;
        createRoomOptions.name = "My Own Room";

        clvClientRealizeCreateRoom(&clientRealize, &createRoomOptions);
    } else {
        ClvSerializeRoomJoinOptions joinRoomOptions;
        joinRoomOptions.name = "My Own Room";

        clvClientRealizeJoinRoom(&clientRealize, &joinRoomOptions);
    }

    ClvClientRealizeState reportedState;
    reportedState = ClvClientRealizeStateInit;
    while (true) {
        usleep(16 * 1000);

        MonotonicTimeMs now = monotonicTimeMsNow();
        clvClientRealizeUpdate(&clientRealize, now);

        if (reportedState != clientRealize.state) {
            reportedState = clientRealize.state;
            switch (clientRealize.state) {
                case ClvClientRealizeStateInit:
                case ClvClientRealizeStateReInit:
                case ClvClientRealizeStateCleared:
                    break;
                case ClvClientRealizeStateCreateRoom:
                    CLOG_NOTICE("CREATED ROOM!")
                    break;
                case ClvClientRealizeStateJoinRoom:
                    CLOG_NOTICE("Joined ROOM!")
                    break;
            }
        }

        switch (clientRealize.state) {
            case ClvClientRealizeStateJoinRoom: {
                clvClientRealizeSendPacket(&clientRealize, 0, "Hello", 6);
                uint8_t inBuf[1200];
                int inConnectionId;
                int received = clvClientRealizeReadPacket(&clientRealize, &inConnectionId, inBuf, 1200);
                if (received > 0) {
                    CLOG_NOTICE("got packet: '%s' %d", inBuf, inConnectionId);
                }
            } break;
            case ClvClientRealizeStateInit:
                break;
            case ClvClientRealizeStateReInit:
                break;
            case ClvClientRealizeStateCleared:
                break;
            case ClvClientRealizeStateCreateRoom:
                break;
        }
    }

    imprintDefaultSetupDestroy(&memory);
}