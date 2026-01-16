# How to Contribute to R-Type Network Server

This guide explains how to contribute to the R-Type server's networking layer, including implementing new message types, modifying the protocol, and extending server functionality.

---

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [Architecture Overview](#architecture-overview)
3. [Network Protocol](#network-protocol)
4. [Adding New Message Types](#adding-new-message-types)
5. [Implementing Server Features](#implementing-server-features)
6. [Testing Your Changes](#testing-your-changes)
7. [Best Practices](#best-practices)
8. [Common Tasks](#common-tasks)
9. [Troubleshooting](#troubleshooting)

---

## Prerequisites

Before contributing to the network server, ensure you have:

- **Understanding of**:
  - C++17 or higher
  - Asynchronous I/O (ASIO library)
  - TCP/UDP networking concepts
  - Binary protocols and serialization
  - Multithreading and thread safety

- **Read the following documentation**:
  - [Server Technical Documentation](SERVER_TECHNICAL_DOC.md)
  - [Network Protocol Specification](network/protocoleRFC.txt)
  - [Full Class Documentation](https://jeremyarmijo.github.io/R-Type/)

- **Development Environment**:
  - GCC 14+ or compatible compiler
  - CMake 3.30+
  - CPM package manager
  - ASIO library (automatically fetched via CPM)

---

## Architecture Overview

### Key Components

The R-Type server networking layer consists of:

1. **ServerNetworkManager** ([Documentation](https://jeremyarmijo.github.io/R-Type/classServerNetworkManager.html))
   - Orchestrates all network operations
   - Manages TCP and UDP servers
   - Routes messages between network and game logic
   - File: `Server/src/network/ServerNetworkManager.hpp/cpp`

2. **TCPServer** ([Documentation](https://jeremyarmijo.github.io/R-Type/classTCPServer.html))
   - Handles reliable client connections
   - Manages authentication and session persistence
   - File: `Server/src/network/TCPServer.hpp/cpp`

3. **UDPServer** ([Documentation](https://jeremyarmijo.github.io/R-Type/classUDPServer.html))
   - Handles fast, unreliable game updates
   - Processes player input and game state
   - File: `Server/src/network/UDPServer.hpp/cpp`

4. **ClientManager** ([Documentation](https://jeremyarmijo.github.io/R-Type/classClientManager.html))
   - Tracks connected clients
   - Associates TCP and UDP endpoints
   - Monitors client timeouts
   - File: `Server/src/network/ClientManager.hpp/cpp`

5. **Encoder/Decoder** (Shared)
   - Serializes/deserializes network messages
   - Files: `Shared/network/Encoder.hpp`, `Shared/network/Decoder.hpp`

### Threading Model

```
┌─────────────┐     ┌──────────────┐     ┌─────────────┐
│  Main       │────▶│  Game Logic  │────▶│  Network    │
│  Thread     │     │  Thread      │     │  I/O Thread │
└─────────────┘     └──────────────┘     └─────────────┘
      │                     │                     │
      │              ┌──────┴──────┐             │
      │              │   Event     │             │
      └──────────────│   Queues    │─────────────┘
                     │  (Mutexed)  │
                     └─────────────┘
```

**Important**: Always use mutexes when accessing shared data structures across threads.

---

## Network Protocol

### Packet Structure

All network packets follow this format:

```
+--------+-------+----------+-------------+
| Type   | Flags | Length   | Payload     |
| (1B)   | (1B)  | (4B)     | (variable)  |
+--------+-------+----------+-------------+
```

- **Type (uint8_t)**: Message type identifier (see `Shared/network/Event.hpp`)
- **Flags (uint8_t)**: Reserved for future use
- **Length (uint32_t)**: Payload size in bytes
- **Payload**: Variable-length message data

### Message Types

See [Network Protocol RFC](network/protocoleRFC.txt) for full list. Common types:

**TCP Messages:**
- `0x01` LOGIN_REQUEST
- `0x02` LOGIN_RESPONSE
- `0x0F` GAME_START

**UDP Messages:**
- `0x10` PLAYER_INPUT
- `0x11` ENTITY_UPDATE
- `0x12` GAME_STATE_UPDATE

---

## Adding New Message Types

### Step 1: Define the Message Type

Add your message type to `Shared/network/Event.hpp`:

```cpp
enum class EventType : uint8_t {
    // Existing types...
    NEW_FEATURE = 0x50,  // Your new message type
};
```

### Step 2: Define the Message Structure

Create a struct in `Shared/network/Event.hpp`:

```cpp
struct NewFeatureEvent {
    uint32_t playerId;
    float value;
    // Add your fields here
};
```

### Step 3: Implement Encoding

Add encoding logic in `Shared/network/EncodeFunc.cpp`:

```cpp
std::vector<uint8_t> encodeNewFeature(const NewFeatureEvent& event) {
    std::vector<uint8_t> data;
    
    // Encode playerId (4 bytes)
    data.push_back((event.playerId >> 24) & 0xFF);
    data.push_back((event.playerId >> 16) & 0xFF);
    data.push_back((event.playerId >> 8) & 0xFF);
    data.push_back(event.playerId & 0xFF);
    
    // Encode value (4 bytes, float to bytes)
    uint32_t floatBits;
    std::memcpy(&floatBits, &event.value, sizeof(float));
    data.push_back((floatBits >> 24) & 0xFF);
    data.push_back((floatBits >> 16) & 0xFF);
    data.push_back((floatBits >> 8) & 0xFF);
    data.push_back(floatBits & 0xFF);
    
    return data;
}
```

Register it in `Encoder.hpp`:

```cpp
class Encoder {
public:
    template<typename T>
    std::vector<uint8_t> Encode(const T& event);
    
    // Add specialization
    template<>
    std::vector<uint8_t> Encode<NewFeatureEvent>(const NewFeatureEvent& event) {
        return encodeNewFeature(event);
    }
};
```

### Step 4: Implement Decoding

Add decoding logic in `Shared/network/DecodeFunc.cpp`:

```cpp
NewFeatureEvent decodeNewFeature(const std::vector<uint8_t>& data, size_t& offset) {
    NewFeatureEvent event;
    
    // Decode playerId
    event.playerId = (static_cast<uint32_t>(data[offset]) << 24) |
                     (static_cast<uint32_t>(data[offset + 1]) << 16) |
                     (static_cast<uint32_t>(data[offset + 2]) << 8) |
                     static_cast<uint32_t>(data[offset + 3]);
    offset += 4;
    
    // Decode value
    uint32_t floatBits = (static_cast<uint32_t>(data[offset]) << 24) |
                         (static_cast<uint32_t>(data[offset + 1]) << 16) |
                         (static_cast<uint32_t>(data[offset + 2]) << 8) |
                         static_cast<uint32_t>(data[offset + 3]);
    std::memcpy(&event.value, &floatBits, sizeof(float));
    offset += 4;
    
    return event;
}
```

### Step 5: Handle the Message on Server

Add handler in `ServerNetworkManager` or appropriate component:

**For TCP messages** (`Server/src/network/TCPServer.cpp`):

```cpp
void TCPServer::ProcessPacket(uint32_t clientId, const std::vector<uint8_t>& data) {
    // Existing code...
    
    switch (type) {
        case static_cast<uint8_t>(EventType::NEW_FEATURE): {
            size_t offset = HEADER_SIZE;
            NewFeatureEvent event = decodeNewFeature(data, offset);
            HandleNewFeature(clientId, event);
            break;
        }
        // Other cases...
    }
}

void TCPServer::HandleNewFeature(uint32_t clientId, const NewFeatureEvent& event) {
    std::cout << "[NewFeature] Client " << clientId 
              << " value: " << event.value << std::endl;
    
    // Implement your logic here
    // Example: Queue event for game logic
    m_eventQueue.Push({EventType::NEW_FEATURE, /* data */});
}
```

**For UDP messages** (`Server/src/network/UDPServer.cpp`):

```cpp
void UDPServer::ProcessPacket(const udp::endpoint& endpoint, 
                               const std::vector<uint8_t>& data) {
    // Similar pattern to TCP
}
```

### Step 6: Update the Protocol Documentation

Document your new message in `documentation/network/protocoleRFC.txt`:

```
Message Type: NEW_FEATURE (0x50)
Direction: Client → Server
Transport: TCP / UDP
Description: Brief description of what this message does

Payload Structure:
+-------------+--------+--------------------------------+
| Field       | Size   | Description                    |
+-------------+--------+--------------------------------+
| playerId    | 4 bytes| ID of the player               |
| value       | 4 bytes| Float value for feature        |
+-------------+--------+--------------------------------+

Example:
Type: 0x50
Flags: 0x00
Length: 0x00000008 (8 bytes)
Payload: [playerId (4B)][value (4B)]
```

---

## Implementing Server Features

### Adding a New Server Command

1. **Define the command handler** in `ServerGame.hpp`:

```cpp
class ServerGame {
private:
    void HandleCustomCommand(const CustomCommandEvent& event);
};
```

2. **Implement the handler** in `ServerGame.cpp`:

```cpp
void ServerGame::HandleCustomCommand(const CustomCommandEvent& event) {
    std::lock_guard<std::mutex> lock(m_lobbyMutex);
    
    // Access game state safely
    auto* lobby = GetLobby(event.lobbyId);
    if (!lobby) return;
    
    // Implement your logic
    // ...
    
    // Send response to clients
    BroadcastToLobby(lobby->id, responseEvent);
}
```

3. **Register in event processing** loop:

```cpp
void ServerGame::ProcessEvents() {
    while (m_running) {
        Event event;
        if (m_eventQueue.TryPop(event)) {
            switch (event.type) {
                case EventType::CUSTOM_COMMAND:
                    HandleCustomCommand(/* decode event */);
                    break;
                // Other events...
            }
        }
    }
}
```

### Modifying Game State Updates

Game state updates are sent periodically via UDP. To modify:

1. **Update the state structure** in `Shared/network/Event.hpp`:

```cpp
struct GameStateUpdate {
    uint32_t lobbyId;
    uint32_t timestamp;
    std::vector<EntityData> entities;
    // Add your new fields
    CustomData customData;
};
```

2. **Update encoding/decoding** in `EncodeFunc.cpp` and `DecodeFunc.cpp`

3. **Modify the update broadcast** in `ServerGame.cpp`:

```cpp
void ServerGame::BroadcastGameState(Lobby& lobby) {
    GameStateUpdate update;
    update.lobbyId = lobby.id;
    update.timestamp = getCurrentTimestamp();
    
    // Populate entity data
    // ...
    
    // Add your custom data
    update.customData = GenerateCustomData(lobby);
    
    // Encode and send
    auto encoded = m_encoder.Encode(update);
    m_networkManager->SendUDPToLobby(lobby.id, encoded);
}
```

---

## Testing Your Changes

### Unit Testing

Create tests in `Server/tests/` (if test infrastructure exists):

```cpp
TEST(NetworkTest, NewFeatureEncoding) {
    NewFeatureEvent event{42, 3.14f};
    auto encoded = encodeNewFeature(event);
    
    size_t offset = 0;
    auto decoded = decodeNewFeature(encoded, offset);
    
    EXPECT_EQ(event.playerId, decoded.playerId);
    EXPECT_FLOAT_EQ(event.value, decoded.value);
}
```

### Integration Testing

1. **Build the server**:
```bash
cd Server/build
cmake ..
make
```

2. **Run with logging enabled**:
```bash
./RTYPE_Server 1 0.0.0.0
```

3. **Use a test client** or packet tool to send your new message

4. **Verify server logs** show correct handling:
```
[NewFeature] Client 1 value: 3.14
```

### Manual Testing Checklist

- [ ] Message is correctly encoded
- [ ] Message is correctly decoded
- [ ] Server handles message without crashes
- [ ] Thread safety is maintained (no data races)
- [ ] Client receives appropriate response
- [ ] Invalid messages are handled gracefully
- [ ] Performance is acceptable (no blocking operations)

---

## Best Practices

### Thread Safety

**Always** protect shared data with mutexes:

```cpp
// ✓ CORRECT
{
    std::lock_guard<std::mutex> lock(m_clientMutex);
    m_clients[clientId] = clientData;
}

// ✗ INCORRECT (data race!)
m_clients[clientId] = clientData;
```

### Error Handling

**Always** validate input data:

```cpp
void ProcessPacket(const std::vector<uint8_t>& data) {
    // Check minimum size
    if (data.size() < HEADER_SIZE) {
        std::cerr << "Packet too small" << std::endl;
        return;
    }
    
    // Validate message type
    uint8_t type = data[0];
    if (type > MAX_MESSAGE_TYPE) {
        std::cerr << "Invalid message type: " << type << std::endl;
        return;
    }
    
    // Validate payload length
    uint32_t payloadLength = extractLength(data);
    if (data.size() != HEADER_SIZE + payloadLength) {
        std::cerr << "Length mismatch" << std::endl;
        return;
    }
    
    // Proceed with processing...
}
```

### Network Efficiency

**Minimize packet size**:

```cpp
// ✓ Use compact data types
struct CompactUpdate {
    uint16_t entityId;  // 0-65535 is enough
    int16_t x, y;       // Fixed-point coordinates
    uint8_t health;     // 0-255
};

// ✗ Avoid large types when not needed
struct WastefulUpdate {
    uint64_t entityId;  // Overkill for entity count
    double x, y;        // Too much precision
    uint32_t health;    // Only need 0-100
};
```

### Logging

Use descriptive log messages:

```cpp
std::cout << "[" << __FUNCTION__ << "] "
          << "Client " << clientId << " "
          << "performed action: " << actionName
          << std::endl;
```

### Code Organization

- **Keep network code in `Server/src/network/`**
- **Keep shared protocol code in `Shared/network/`**
- **Document all public functions** (use Doxygen format)
- **Follow existing naming conventions**

---

## Common Tasks

### Task 1: Add Client Timeout Handling

```cpp
// In ClientManager.cpp
void ClientManager::CheckTimeouts() {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto now = std::chrono::steady_clock::now();
    
    for (auto it = m_clients.begin(); it != m_clients.end();) {
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - it->second.lastActivity).count();
        
        if (elapsed > TIMEOUT_SECONDS) {
            std::cout << "[Timeout] Client " << it->first << std::endl;
            it = m_clients.erase(it);
        } else {
            ++it;
        }
    }
}
```

### Task 2: Implement Message Rate Limiting

```cpp
class RateLimiter {
private:
    struct ClientRate {
        std::chrono::steady_clock::time_point lastReset;
        uint32_t messageCount;
    };
    
    std::unordered_map<uint32_t, ClientRate> m_rates;
    uint32_t m_maxMessagesPerSecond = 60;
    
public:
    bool CheckLimit(uint32_t clientId) {
        auto now = std::chrono::steady_clock::now();
        auto& rate = m_rates[clientId];
        
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
            now - rate.lastReset).count();
        
        if (elapsed >= 1) {
            rate.lastReset = now;
            rate.messageCount = 0;
        }
        
        if (rate.messageCount >= m_maxMessagesPerSecond) {
            return false;  // Rate limit exceeded
        }
        
        rate.messageCount++;
        return true;
    }
};
```

### Task 3: Add Network Statistics

```cpp
struct NetworkStats {
    uint64_t packetsSent = 0;
    uint64_t packetsReceived = 0;
    uint64_t bytesSent = 0;
    uint64_t bytesReceived = 0;
    
    void PrintStats() const {
        std::cout << "\n=== Network Statistics ===" << std::endl;
        std::cout << "Packets sent: " << packetsSent << std::endl;
        std::cout << "Packets received: " << packetsReceived << std::endl;
        std::cout << "Bytes sent: " << bytesSent << std::endl;
        std::cout << "Bytes received: " << bytesReceived << std::endl;
        std::cout << "========================\n" << std::endl;
    }
};
```

---

## Troubleshooting

### Issue: "Packet size mismatch"

**Cause**: Encoding/decoding inconsistency

**Solution**: Verify that encoding and decoding use the same byte order and field order:

```cpp
// Encoding
data.push_back((value >> 8) & 0xFF);   // High byte first
data.push_back(value & 0xFF);          // Low byte second

// Decoding (must match!)
value = (data[offset] << 8) | data[offset + 1];
```

### Issue: "Segmentation fault in network thread"

**Cause**: Accessing freed memory or data race

**Solution**:
1. Use Valgrind: `valgrind ./RTYPE_Server`
2. Add thread sanitizer: `cmake -DCMAKE_CXX_FLAGS="-fsanitize=thread" ..`
3. Review all mutex usage

### Issue: "Client not receiving UDP packets"

**Cause**: UDP endpoint not associated

**Solution**: Ensure client sends first UDP packet before server tries to respond:

```cpp
// In UDPServer::ProcessPacket
if (!m_clientManager->IsUDPAssociated(clientId)) {
    m_clientManager->AssociateUDP(clientId, senderEndpoint);
    std::cout << "[UDP] Associated client " << clientId << std::endl;
}
```

### Issue: "High CPU usage"

**Cause**: Busy-waiting in network loop

**Solution**: Use async operations with proper wait conditions:

```cpp
// ✗ WRONG: Busy loop
while (running) {
    if (HasData()) ProcessData();
}

// ✓ CORRECT: Async with blocking
m_socket.async_receive_from(
    asio::buffer(m_buffer), m_remoteEndpoint,
    [this](const error_code& ec, size_t bytes) {
        if (!ec) HandleReceive(bytes);
        StartReceive();  // Chain next async receive
    });

m_ioContext.run();  // Blocks until work available
```

---

## Additional Resources

- **[Doxygen Documentation](https://jeremyarmijo.github.io/R-Type/)** - Complete API reference
- **[ASIO Documentation](https://think-async.com/Asio/)** - Networking library reference
- **[Server Technical Doc](SERVER_TECHNICAL_DOC.md)** - Architecture overview
- **[Protocol RFC](network/protocoleRFC.txt)** - Network protocol specification
- **[Contributing Guidelines](CONTRIBUTING.md)** - General contribution rules

---

## Getting Help

If you need assistance:

1. **Check existing documentation** first
2. **Review similar implementations** in the codebase
3. **Ask on GitHub Issues** with:
   - What you're trying to implement
   - What you've tried
   - Error messages or unexpected behavior
   - Relevant code snippets

---

## Summary

Contributing to the R-Type network server involves:

1. **Understanding the architecture** - Read the docs!
2. **Defining your changes** - Message types, handlers, features
3. **Implementing carefully** - Thread safety, validation, efficiency
4. **Testing thoroughly** - Unit tests, integration tests, manual verification
5. **Documenting properly** - Update protocol docs, add code comments
6. **Following best practices** - Code style, error handling, logging

Thank you for contributing to R-Type!
