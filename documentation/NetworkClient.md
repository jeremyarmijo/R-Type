# Architecture Réseau Client R-Type

## Structure du Projet

```
Network/
├── include/
│   ├── Action.hpp          # Actions client → serveur (mouvements, requêtes)
│   ├── Event.hpp           # Événements serveur → client (réponses, états)
│   ├── Encoder.hpp         # Interface d'encodage Actions → paquets binaires
│   ├── Decoder.hpp         # Interface de décodage paquets → Events
│   ├── EncodeFunc.hpp      # Handlers d'encodage par type d'action
│   ├── DecodFunc.hpp       # Handlers de décodage par type d'événement
│   ├── NetworkManager.hpp  # Orchestrateur TCP/UDP + threading + buffers
│   └── CircularBuffer.hpp  # Buffer circulaire thread-safe générique
└── src/
    ├── Encoder.cpp         # Implémentation encodeur + registry handlers
    ├── Decoder.cpp         # Implémentation décodeur + registry handlers
    └── NetworkManager.cpp  # Gestion connexions, I/O réseau, threading
```

**Vue d'ensemble** : Architecture client réseau modulaire avec séparation stricte des responsabilités. `NetworkManager` centralise la logique réseau, `Encoder/Decoder` gèrent la sérialisation, `CircularBuffer` assure la communication thread-safe. Les handlers sont découplés du code central via un pattern Registry.

---

## 1. Architecture Double Canal TCP/UDP

Le système maintient deux connexions simultanées : TCP pour fiabilité (authentification, lobbies, chunks), UDP pour latence minimale (inputs, états temps-réel). La séparation garantit que le gameplay n'est jamais bloqué par des transferts lourds.

**Séquence de connexion** :
```cpp
// NetworkManager.cpp - ThreadLoop()
while (running) {
  if (!tcpConnected) {
    ConnectTCP();  // Première connexion
  }
  if (!udpConnected && udpPort != -1) {
    ConnectUDP();  // Après réception udpPort dans LOGIN_RESPONSE
  }
  // ...
}
```

Chaque action possède un protocole prédéfini via `UseUdp()` :
```cpp
// Action.hpp
inline size_t UseUdp(ActionType type) {
  switch (type) {
    case ActionType::UP:
    case ActionType::SHOOT:
      return 0;  // UDP sans ACK
    case ActionType::LOGIN_REQUEST:
      return 2;  // TCP
    default:
      return 3;  // Invalide
  }
}
```

Le serveur transmet le port UDP dans `LOGIN_RESPONSE.udpPort`, déclenchant automatiquement la connexion UDP.

---

## 2. Threading et Synchronisation

Un thread dédié isole les opérations I/O du thread principal, évitant les blocages sur lecture/écriture réseau. Communication via `CircularBuffer` protégé par mutex pour garantir la cohérence des données.

**Architecture des threads** :
```
Thread Principal              Thread Réseau (networkThread)
     │                                 │
     ├─ SendAction(action) ────────►  ├─ actionBuffer.pop()
     │                                 ├─ Encoder.encode()
     │                                 ├─ SendUdp() / SendTcp()
     │                                 │
     ├◄──── PopEvent() ───────────────┤
                                       ├─ ReadTCP() / ReadUDP()
                                       ├─ Decoder.decode()
                                       └─ eventBuffer.push()
```

**Boucle réseau** :
```cpp
void NetworkManager::ThreadLoop() {
  while (running) {
    if (tcpConnected) ReadTCP();
    if (udpConnected) ReadUDP();
    SendActionServer();  // Vide actionBuffer
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
```

**Interface thread-safe** :
```cpp
void NetworkManager::SendAction(Action action) {
  std::lock_guard<std::mutex> lock(mut);
  actionBuffer.push(action);  // Thread-safe
}

Event NetworkManager::PopEvent() {
  std::lock_guard<std::mutex> lock(mut);
  auto res = eventBuffer.pop();
  return res.has_value() ? res.value() : Event{EventType::UNKNOWN};
}
```

---

## 3. Système de Sérialisation Pattern Registry

Encoder et Decoder utilisent des tables de handlers indexées par type de message, permettant l'ajout dynamique de nouveaux types sans modifier le code central.

**Enregistrement des handlers** :
```cpp
// EncodeFunc.hpp - SetupEncoder()
void SetupEncoder(Encoder& encoder) {
  encoder.registerHandler(ActionType::UP, Player_Up);
  encoder.registerHandler(ActionType::SHOOT, Player_Shoot);
  encoder.registerHandler(ActionType::LOGIN_REQUEST, LoginRequest);
  // ...
}

// DecodFunc.hpp - SetupDecoder()
void SetupDecoder(Decoder& decoder) {
  decoder.registerHandler(0x02, DecodeLOGIN_RESPONSE);
  decoder.registerHandler(0x07, DecodeLOBBY_LIST_RESPONSE);
  // ...
}
```

**Mécanisme d'encodage** :
```cpp
// Encoder.cpp
std::vector<uint8_t> Encoder::encode(const Action& a, size_t useUDP, uint32_t sequenceNum) {
  auto& func = handlers[static_cast<uint8_t>(a.type)];
  if (!func) return {};
  
  std::vector<uint8_t> payload;
  func(a, payload, sequenceNum);  // Handler spécifique
  
  // Ajout header
  PacketHeader header;
  header.type = static_cast<uint8_t>(a.type);
  header.flags = (useUDP == 0) ? 0x02 : 0x01;
  header.length = payload.size();
  
  std::vector<uint8_t> packet;
  writeHeader(packet, header);
  packet.insert(packet.end(), payload.begin(), payload.end());
  return packet;
}
```

**Exemple de handler d'encodage** :
```cpp
void Player_Up(const Action& a, std::vector<uint8_t>& out, uint32_t sequenceNum) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;
  
  out.resize(11);
  size_t offset = 0;
  
  memcpy(out.data() + offset, &sequenceNum, sizeof(uint32_t));
  offset += sizeof(uint32_t);
  memcpy(out.data() + offset, &input->tick, sizeof(uint32_t));
  offset += sizeof(uint32_t);
  
  int8_t mx = 0, my = -1;
  memcpy(out.data() + offset, &mx, sizeof(int8_t));
  offset += sizeof(int8_t);
  memcpy(out.data() + offset, &my, sizeof(int8_t));
  offset += sizeof(int8_t);
  
  uint8_t actions = 0;
  memcpy(out.data() + offset, &actions, sizeof(uint8_t));
}
```

---

## 4. Format des Paquets et Sérialisation Binaire

Tous les paquets suivent un format unifié : header 6 bytes + payload variable. Sérialisation via `memcpy` pour performance maximale sans overhead de streams.

**Structure header** :
```cpp
struct PacketHeader {
  uint8_t type;      // Type de message (0x01-0x2D)
  uint8_t flags;     // 0x01=TCP, 0x02=UDP, 0x08=RequiresACK
  uint32_t length;   // Taille payload (little-endian)
};
```

**Écriture header** :
```cpp
void Encoder::writeHeader(std::vector<uint8_t>& packet, const PacketHeader& h) {
  packet.push_back(h.type);
  packet.push_back(h.flags);
  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&h.length);
  packet.insert(packet.end(), ptr, ptr + 4);
}
```

Les tailles fixes (ex: `PLAYER_INPUT` = 11 bytes) permettent des optimisations d'allocation. Les strings utilisent un préfixe de longueur (uint8/uint16).

---

## 5. Gestion des Connexions et Reconnexion

Sockets configurés en mode non-bloquant pour éviter les blocages du thread réseau. Reconnexion automatique TCP si déconnexion détectée. UDP ne se connecte qu'après réception du port dans `LOGIN_RESPONSE`.

**Connexion TCP non-bloquante** :
```cpp
int NetworkManager::ConnectTCP() {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  // ... configuration adresse ...
  
  if (connect(sockfd, ...) < 0) {
    std::cerr << "Connexion TCP error\n";
    return -1;
  }
  
  tcpSocket = sockfd;
  int flags = fcntl(tcpSocket, F_GETFL, 0);
  fcntl(tcpSocket, F_SETFL, flags | O_NONBLOCK);  // Non-bloquant
  
  tcpConnected = true;
  return 0;
}
```

**Gestion erreurs lecture** :
```cpp
void NetworkManager::ReadTCP() {
  char tempBuffer[1024];
  int bytesReceived = recv(tcpSocket, tempBuffer, sizeof(tempBuffer), 0);
  
  if (bytesReceived > 0) {
    recvTcpBuffer.insert(recvTcpBuffer.end(), tempBuffer, tempBuffer + bytesReceived);
    ProcessTCPRecvBuffer();
  } else if (bytesReceived == 0) {
    tcpConnected = false;  // Déconnexion propre
  } else {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return;  // Normal pour non-bloquant
    }
    tcpConnected = false;  // Erreur réseau
  }
}
```

**Reconstruction paquets TCP** :
```cpp
void NetworkManager::ProcessTCPRecvBuffer() {
  while (recvTcpBuffer.size() >= 6) {
    uint32_t packetSize;
    memcpy(&packetSize, &recvTcpBuffer[2], sizeof(packetSize));
    
    if (recvTcpBuffer.size() < 6 + packetSize) return;  // Paquet incomplet
    
    std::vector<uint8_t> packet(recvTcpBuffer.begin(), recvTcpBuffer.begin() + 6 + packetSize);
    recvTcpBuffer.erase(recvTcpBuffer.begin(), recvTcpBuffer.begin() + 6 + packetSize);
    
    Event evt = DecodePacket(packet);
    eventBuffer.push(evt);
  }
}
```

---

## 6. CircularBuffer Thread-Safe Optimisé

Buffer de taille fixe avec indices circulaires évitant les allocations dynamiques. Écrasement automatique des anciennes données si plein, garantissant que les nouvelles données critiques passent toujours.

**Implémentation** :
```cpp
template <typename T>
class CircularBuffer {
private:
  std::vector<T> m_buffer;
  size_t m_maxItem;
  size_t m_lastItemIndex = 0;
  size_t m_firstItemIndex = 0;
  size_t m_nbItem = 0;

public:
  explicit CircularBuffer(size_t capacity) : m_maxItem(capacity), m_buffer(capacity) {}
  
  void push(const T& item) {
    m_buffer[m_lastItemIndex] = item;
    m_lastItemIndex = (m_lastItemIndex + 1) % m_maxItem;
    
    if (m_nbItem < m_maxItem) {
      m_nbItem++;
    } else {
      m_firstItemIndex = (m_firstItemIndex + 1) % m_maxItem;  // Écrase ancien
    }
  }
  
  std::optional<T> pop() {
    if (m_nbItem == 0) return std::nullopt;
    
    T item = m_buffer[m_firstItemIndex];
    m_firstItemIndex = (m_firstItemIndex + 1) % m_maxItem;
    m_nbItem--;
    return item;
  }
};
```

**Utilisation dans NetworkManager** :
```cpp
class NetworkManager {
  CircularBuffer<Event> eventBuffer{50};   // 50 événements max
  CircularBuffer<Action> actionBuffer{50}; // 50 actions max
  std::mutex mut;  // Protège les deux buffers
};
```

---

## 7. Variants

`std::variant` remplace les unions C pour représenter des données hétérogènes avec vérification de type à la compilation. `std::get_if` permet l'extraction sûre avec fallback si mauvais type.

**Définitions** :
```cpp
// Action.hpp
using ActionData = std::variant
  std::monostate,
  PlayerInput,
  LoginData,
  SignupData,
  LogoutData,
  // ...
>;

struct Action {
  ActionType type;
  ActionData data;
};

// Event.hpp
using EventData = std::variant
  std::monostate,
  LOGIN_RESPONSE,
  LOBBY_LIST_RESPONSE,
  GAME_STATE,
  // ...
>;

struct Event {
  EventType type;
  EventData data;
};
```

**Extraction type-safe** :
```cpp
void Player_Up(const Action& a, std::vector<uint8_t>& out, uint32_t sequenceNum) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;  // Protection si mauvais type
  
  // Utilisation sûre de input->tick, etc.
}
```

---

## 8. Mécanisme d'ACK UDP pour Messages Critiques

Certains messages UDP portent le flag `0x08` (RequiresACK), déclenchant l'envoi automatique d'un accusé de réception. Le serveur peut ainsi détecter les pertes sans implémenter une fiabilité complète sur UDP.

**Envoi automatique d'ACK** :
```cpp
void NetworkManager::SendACK(std::vector<uint8_t>& evt) {
  if (evt.size() < 6) return;
  
  uint8_t flag;
  memcpy(&flag, &evt[1], sizeof(flag));
  if (flag != 0x08) return;  // Pas besoin d'ACK
  
  uint32_t sequenceNum;
  memcpy(&sequenceNum, &evt[6], sizeof(sequenceNum));
  
  std::vector<uint8_t> response(11);
  response[0] = 0x2A;  // Type ACK
  response[1] = 0x02;  // Flag UDP
  uint32_t payloadSize = 5;
  memcpy(&response[2], &payloadSize, sizeof(uint32_t));
  memcpy(&response[6], &sequenceNum, sizeof(uint32_t));
  response[10] = evt[0];  // Type message original
  
  SendUdp(response);
}

void NetworkManager::ReadUDP() {
  char tempBuffer[2048];
  int bytesReceived = recv(udpSocket, tempBuffer, sizeof(tempBuffer), 0);
  
  if (bytesReceived > 0) {
    std::vector<uint8_t> packet(tempBuffer, tempBuffer + bytesReceived);
    SendACK(packet);  // ACK automatique si nécessaire
    Event evt = DecodePacket(packet);
    eventBuffer.push(evt);
  }
}
```

---

## 9. Extensibilité et Ajout de Nouveaux Messages

L'architecture découplée facilite l'ajout de nouveaux types de messages en 4 étapes simples :

**1. Définir l'enum** :
```cpp
// Action.hpp ou Event.hpp
enum class ActionType : uint8_t {
  // ...
  NEW_ACTION = 0x30,
};
```

**2. Créer la structure de données** :
```cpp
struct NewActionData {
  uint32_t field1;
  std::string field2;
};

// Ajouter au variant
using ActionData = std::variant
  // ...
  NewActionData
>;
```

**3. Implémenter le handler** :
```cpp
void EncodeNewAction(const Action& a, std::vector<uint8_t>& out, uint32_t seq) {
  const auto* data = std::get_if<NewActionData>(&a.data);
  if (!data) return;
  
  size_t offset = 0;
  // Sérialisation...
}
```

**4. Enregistrer** :
```cpp
void SetupEncoder(Encoder& encoder) {
  // ...
  encoder.registerHandler(ActionType::NEW_ACTION, EncodeNewAction);
}
```

Le système de flags permet également d'ajouter des fonctionnalités (compression, chiffrement) sans casser la compatibilité.
