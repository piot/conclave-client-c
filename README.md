<div align="center">
    <img src="https://github.com/piot/conclave/raw/main/docs/images/logo.svg" width="192" />
</div>

# Conclave Client

Connects to a [Conclave Server](https://github.com/piot/conclave-server-lib) using the [Conclave Protocol](https://github.com/piot/conclave-serialize-c).

> [!IMPORTANT]
> Designed for internal development and testing, do not use in production.

## Usage

### Initialize

```c
typedef struct ClvClientRealizeSettings {
    UdpTransportInOut transport;
    const char* username;
    struct ImprintAllocator* memory;
} ClvClientRealizeSettings;

void clvClientRealizeInit(ClvClientRealize* self, const ClvClientRealizeSettings* settings);
```

### Update

```c
void clvClientRealizeUpdate(ClvClientRealize* self, MonotonicTimeMs now);
```

### Create Room

```c
typedef struct ClvSerializeRoomCreateOptions {
    size_t maxNumberOfPlayers;
    int flags;
    const char* name;
} ClvSerializeRoomCreateOptions;

void clvClientRealizeCreateRoom(ClvClientRealize* self,
                                const ClvSerializeRoomCreateOptions* roomOptions);
```

### List Rooms

```c
typedef struct ClvSerializeListRoomsOptions {
    ClvSerializeApplicationId applicationId;
    uint8_t maximumCount;
} ClvSerializeListRoomsOptions;

void clvClientRealizeListRooms(ClvClientRealize* self,
                               const ClvSerializeListRoomsOptions* listRooms);
```

### Join Room

```c
typedef struct ClvSerializeRoomJoinOptions {
    ClvSerializeRoomId roomIdToJoin;
} ClvSerializeRoomJoinOptions;

void clvClientRealizeJoinRoom(ClvClientRealize* self, const ClvSerializeRoomJoinOptions* joinRoom);
```
