/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/conclave-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
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

static ssize_t clientReceive(void* _self, uint8_t* data, size_t size)
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

    DatagramTransport transportInOut;

    imprintDefaultSetupInit(&memory, 16 * 1024 * 1024);

    int startupErr = udpClientStartup();
    if (startupErr < 0) {
        return startupErr;
    }

    const char* hostToConnectTo = "127.0.0.1";

    if (argc > 1) {
        hostToConnectTo = argv[1];
    }

    UdpClientSocket udpClientSocket;
    udpClientInit(&udpClientSocket, hostToConnectTo, 27003);

    transportInOut.self = &udpClientSocket;
    transportInOut.receive = clientReceive;
    transportInOut.send = clientSend;

    settings.memory = &memory.tagAllocator.info;
    settings.transport = transportInOut;
    settings.username = "Piot";
    Clog clvClientLog;
    clvClientLog.config = &g_clog;
    clvClientLog.constantPrefix = "ClvClient";
    settings.log = clvClientLog;

    clvClientRealizeInit(&clientRealize, &settings);
    clvClientRealizeReInit(&clientRealize, &settings);

    int operation = 2;

    switch (operation) {
        case 0: {
            ClvSerializeRoomCreateOptions createRoomOptions;
            createRoomOptions.flags = 0;
            createRoomOptions.maxNumberOfPlayers = 4;
            createRoomOptions.name = "My Own Room";

            clvClientRealizeCreateRoom(&clientRealize, &createRoomOptions);
        } break;
        case 1: {
            ClvSerializeRoomJoinOptions joinRoomOptions;
            joinRoomOptions.roomIdToJoin = 1;

            clvClientRealizeJoinRoom(&clientRealize, &joinRoomOptions);
        } break;
        case 2: {
            ClvSerializeListRoomsOptions listRoomsOptions;
            listRoomsOptions.applicationId = 42;
            listRoomsOptions.maximumCount = 5;
            clvClientRealizeListRooms(&clientRealize, &listRoomsOptions);
        } break;
    }

    ClvClientRealizeState reportedState;
    reportedState = ClvClientRealizeStateInit;
    uint16_t packetCount = 0;
    while (true) {
        usleep(16 * 1000);

        MonotonicTimeMs now = monotonicTimeMsNow();
        clvClientRealizeUpdate(&clientRealize, now);

        if (reportedState != clientRealize.state) {
            reportedState = clientRealize.state;
            switch (clientRealize.state) {
                case ClvClientRealizeStateCreateRoom:
                    CLOG_NOTICE("CREATED ROOM!")
                    break;
                case ClvClientRealizeStateJoinRoom:
                    CLOG_NOTICE("Joined ROOM!")
                    break;
                case ClvClientRealizeStateListRoomsDone: {
                    CLOG_NOTICE("Rooms Done!")
                    if (clientRealize.client.listRoomsResponseOptions.roomInfoCount > 0) {
                        CLOG_NOTICE("Joining first room")
                        ClvSerializeRoomJoinOptions joinRoomOptions;
                        joinRoomOptions.roomIdToJoin = clientRealize.client.listRoomsResponseOptions.roomInfos[0]
                                                           .roomId;
                        clvClientRealizeJoinRoom(&clientRealize, &joinRoomOptions);
                    }
                    break;
                }
            }
        }

        if (clientRealize.isInRoom) {
            uint8_t outBuf[32];
            tc_snprintf(outBuf, 32, "Hello %04X", packetCount);
            CLOG_INFO("sent: '%s'", outBuf)
            clvClientRealizeSendPacket(&clientRealize, 0, outBuf, 11);
            packetCount++;
            if (true) {
                uint8_t inBuf[1200];
                int inConnectionId;
                for (size_t i = 0; i < 30; ++i) {
                    int received = clvClientRealizeReadPacket(&clientRealize, &inConnectionId, inBuf, 1200);
                    if (received > 0) {
                        CLOG_NOTICE("got packet: '%s' %d", inBuf, inConnectionId);
                    }
                }
            }
        }
    }

    imprintDefaultSetupDestroy(&memory);
}
