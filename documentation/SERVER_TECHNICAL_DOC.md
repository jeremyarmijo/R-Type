# Server Technical Documentation

## Overview

The R-Type server is a **multithreaded game server** built with C++17 that handles real-time multiplayer gameplay. It uses a hybrid network architecture combining **TCP for reliable authentication** and **UDP for low-latency game state updates**.

---

## Architecture

### Core Components

1. **ServerGame** - Main game logic controller
   - Manages game loop and state
   - Coordinates ECS (Entity Component System)
   - Handles player lobby and game lifecycle

2. **ServerNetworkManager** - Network orchestration layer
   - Manages TCP and UDP servers
   - Routes messages between network and game logic
   - Handles client connection/disconnection events

3. **TCPServer** - Reliable connection handler
   - Client authentication and login
   - Session management
   - Persistent connection state

4. **UDPServer** - Real-time data transmission
   - Fast, unreliable game state updates
   - Player input processing
   - Low-latency communication

5. **ClientManager** - Client state management
   - Tracks connected clients
   - Associates TCP and UDP endpoints
   - Monitors timeouts and disconnections

---

## Network Protocol

### TCP (Port 4242)
- **Purpose**: Client authentication, lobby management
- **Messages**:
  - `LOGIN_REQUEST` (0x01): Client login with username
  - `LOGIN_RESPONSE` (0x02): Server response with player ID and UDP port
  - `GAME_START` (0x0F): Game initialization signal

### UDP (Port 4243)
- **Purpose**: Real-time game data
- **Messages**:
  - Player input events
  - Entity position updates
  - Game state synchronization

### Packet Structure
```
+--------+-------+----------+-------------+
| Type   | Flags | Length   | Payload     |
| (1B)   | (1B)  | (4B)     | (variable)  |
+--------+-------+----------+-------------+
```

---

## ECS Architecture

The server uses an **Entity Component System** for game entities:

- **Components**: Data-only structures (Position, Velocity, Health, etc.)
- **Systems**: Logic that operates on components (PhysicsSystem, CollisionSystem, etc.)
- **Registry**: Central manager for entities and components

### Key Systems
- `PhysicsSystem`: Updates entity positions based on velocity
- `CollisionSystem`: Detects and resolves collisions
- `WeaponSystem`: Handles projectile firing
- `WaveSystem`: Manages enemy spawn waves
- `BoundsSystem`: Removes out-of-bounds entities

---

## Threading Model

The server uses multiple threads for concurrent operations:

1. **Main Thread**: Game loop (60 FPS tick rate)
2. **Network I/O Thread**: ASIO event loop for TCP/UDP
3. **Game Thread**: Separate thread for game state updates

### Synchronization
- **Mutexes**: Protect shared data (event queues, client lists)
- **Thread-safe queues**: Communication between threads
- **Lock guards**: RAII-based locking

---

## Configuration

### Server Initialization
```cpp
ServerGame server;
server.Initialize(
    4242,           // TCP port
    4243,           // UDP port
    1,              // Difficulty level
    "0.0.0.0"       // Host IP (all interfaces)
);
server.Run();
```

### Command Line Arguments
```bash
./r-type_server [difficulty] [host_ip]
# Examples:
./r-type_server                    # Default: difficulty 1, 0.0.0.0
./r-type_server 2                  # Difficulty 2, 0.0.0.0
./r-type_server 1 192.168.1.88     # Difficulty 1, specific IP
```

---

## Client Flow

1. **Connection**: Client connects to TCP server
2. **Authentication**: Sends `LOGIN_REQUEST` with username
3. **Response**: Server assigns player ID, sends UDP port
4. **UDP Association**: Client sends first UDP packet to associate endpoint
5. **Lobby**: Wait for 4 players (or game start)
6. **Game Loop**: Real-time UDP communication for gameplay
7. **Disconnection**: TCP close or timeout triggers cleanup

---

## Error Handling

### Network Errors
- **TCP disconnection**: Automatic client cleanup
- **UDP packet loss**: Expected, no retransmission
- **Timeout**: Clients inactive for 10s are disconnected

### Game Errors
- Invalid message types are logged and ignored
- Malformed packets trigger error logs
- Exception handling prevents server crashes

---

## Performance Considerations

### Optimizations
- **UDP for game data**: Reduces latency vs TCP
- **Binary protocol**: Compact, efficient serialization
- **Spatial partitioning**: (Future) For collision detection
- **Object pooling**: (Future) Reduce allocations

### Scalability
- Current limit: **4 players per game**
- Single-threaded game logic (sufficient for 4 players)
- ASIO handles network I/O efficiently

---

### Key Classes
- [`ServerGame`](https://jeremyarmijo.github.io/R-Type/classServerGame.html) - Main server controller
- [`ServerNetworkManager`](https://jeremyarmijo.github.io/R-Type/classServerNetworkManager.html) - Network management
- [`TCPServer`](https://jeremyarmijo.github.io/R-Type/classTCPServer.html) - TCP connection handler
- [`UDPServer`](https://jeremyarmijo.github.io/R-Type/classUDPServer.html) - UDP communication
- [`ClientManager`](https://jeremyarmijo.github.io/R-Type/classClientManager.html) - Client state tracking

---

## Debugging

### Logging
The server outputs detailed logs to `stdout`:
```
[TCPServer] Listening on 0.0.0.0:4242
[UDPServer] Listening on 0.0.0.0:4243
[ProcessPacketTCP] New connection from 192.168.1.100:54321 (Client ID: 1)
[ServerNetworkManager] Client 1 authenticated: PlayerOne
```


## Building from Source

See [CONAN_SETUP.md](../CONAN_SETUP.md) for detailed build instructions.

### Quick Build
```bash
cd Server
conan install . --output-folder=build --build=missing -s compiler.cppstd=17
cmake -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake
cmake --build build
```

---


## Related Documentation

- [Network Protocol Specification](protocol.md)
- [Full class Documentation](https://jeremyarmijo.github.io/R-Type/)

---

## Support

For questions or issues, please:
- Check the [Doxygen documentation](https://jeremyarmijo.github.io/R-Type/)
- Open an issue on [GitHub](https://github.com/jeremyarmijo/R-Type/issues)
- Review existing documentation in `/documentation`
