# Documentation du Client Réseau R-Type - Structure Actualisée

## Structure du Projet

```
Shared/
├── Action.hpp           # Actions client → serveur
├── Event.hpp           # Événements serveur → client
├── Encoder.hpp         # Interface d'encodage
├── Encoder.cpp         # Implémentation encodeur
├── EncodeFunc.hpp      # Handlers d'encodage spécifiques
├── Decoder.hpp         # Interface de décodage
├── Decoder.cpp         # Implémentation décodeur
├── DecodeFunc.hpp      # Handlers de décodage spécifiques
├── DecodeFunc.cpp      # Implémentation des décodeurs

Network/
├── include/
│   ├── NetworkManager.hpp  # Gestionnaire réseau principal
│   └── CircularBuffer.hpp  # Buffer circulaire thread-safe
└── NetworkManager.cpp      # Implémentation du gestionnaire
```

---

## 1. Architecture Client-Serveur

### 1.1 Vision d'Ensemble
Le client réseau utilise une architecture **double-protocole** :
- **TCP** (fiable) : Authentification, état jeu, messages système
- **UDP** (rapide) : Entrées joueur, mises à jour temps-réel

```
Thread Principal
    ↓
NetworkManager (thread dédié)
    ├── TCP Socket ←→ Décoder → Événements
    └── UDP Socket ←→ Encoder → Actions
```

### 1.2 Séquence de Connexion
1. Connexion TCP au serveur
2. Envoi LOGIN_REQUEST via TCP
3. Réception LOGIN_RESPONSE avec playerId + udpPort
4. Connexion UDP sur le port spécifié
5. Envoi AUTH via UDP pour validation
6. Communication jeu normale

---

## 2. Structures de Données Clés

### 2.1 Action.hpp - Messages Client → Serveur

```cpp
// Enumération des types d'actions
enum class ActionType : uint8_t {
  // Authentification
  AUTH,                   // UDP - Validation de connexion
  LOGIN_REQUEST,          // TCP - Requête de connexion
  
  // Entrées joueur (UDP)
  UP_PRESS, 
  UP_RELEASE, 
  //...

  // Messages serveur → client (utilisés pour l'encodage des réponses)
  LOGIN_RESPONSE, 
  GAME_START,
  //...
};

// Structure pour les entrées joueur
struct PlayerInput {
  bool up, down, left, right;
  uint8_t fire;  // 0 = pas de tir, 1 = tir
};

// Variant contenant tous les types de données
using ActionData = std::variant<std::monostate, 
                               AuthUDP, LoginReq, PlayerInput,
                               LoginResponse, GameStart, GameEnd,
                               ErrorMsg, GameState, BossSpawn,
                               BossUpdate, EnemyHit>;

// Conteneur principal
struct Action {
  ActionType type;
  ActionData data;
};

// Détermine si une action utilise UDP ou TCP
inline size_t UseUdp(ActionType type) {
  switch (type) {
    case ActionType::AUTH:
    case ActionType::UP_PRESS:  // et toutes les autres entrées
    case ActionType::GAME_STATE:
    case ActionType::BOSS_SPAWN:
    case ActionType::BOSS_UPDATE:
    case ActionType::ENEMY_HIT:
      return 0;  // UDP
      
    case ActionType::LOGIN_REQUEST:
    case ActionType::LOGIN_RESPONSE:
    case ActionType::GAME_START:
    case ActionType::GAME_END:
    case ActionType::ERROR:
      return 2;  // TCP
      
    default:
      return 3;  // Invalide
  }
}
```

### 2.2 Event.hpp - Messages Serveur → Client

```cpp
// Enumération des types d'événements
enum class EventType : uint8_t {
  LOGIN_REQUEST = 0x01,
  GAME_START = 0x0F,
  ERROR = 0x12,
  GAME_STATE = 0x21,
  BOSS_SPAWN = 0x23,
  ENEMY_HIT = 0x25,
  // ...
};

// Exemple de structure de données
struct LOGIN_RESPONSE {
  uint8_t success;        // 1 = succès, 0 = échec
  uint16_t playerId;      // ID unique assigné
  uint16_t udpPort;       // Port UDP pour le jeu
  uint16_t errorCode;     // Code d'erreur si échec
  std::string message;    // Message d'erreur/succès
};

// Variant pour tous les événements
using EventData = std::variant<std::monostate,
                              LOGIN_REQUEST, LOGIN_RESPONSE,
                              GAME_START, GAME_END, ERROR,
                              PLAYER_INPUT, GAME_STATE, AUTH,
                              BOSS_SPAWN, BOSS_UPDATE, ENEMY_HIT>;

struct Event {
  EventType type;
  EventData data;
};
```

---

### 3.2 Encoder - Client → Serveur

**Encoder.hpp** :
```cpp
struct PacketHeader {
  uint8_t type;
  uint8_t flags;
  uint32_t length;  // network byte order
};

class Encoder {
public:
  using EncodePayload = std::function<void(const Action&, std::vector<uint8_t>&)>;
  
  Encoder();
  void registerHandler(ActionType type, EncodePayload f);
  std::vector<uint8_t> encode(const Action& a, size_t useUDP);
  
private:
  std::array<EncodePayload, 256> handlers;
  void writeHeader(std::vector<uint8_t>& packet, const PacketHeader& h);
};
```

**EncodeFunc.hpp - Exemple de handler** :
```cpp
inline void PlayerInputFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* input = std::get_if<PlayerInput>(&a.data);
  if (!input) return;
  
  out.resize(5);
  out[0] = input->up ? 1 : 0;
  out[1] = input->down ? 1 : 0;
  out[2] = input->left ? 1 : 0;
  out[3] = input->right ? 1 : 0;
  out[4] = input->fire ? 0x01 : 0x00;
}
```

**Encoder.cpp - Mapping types → IDs** :
```cpp
inline uint8_t getType(const Action& a) {
  switch (a.type) {
    case ActionType::LOGIN_REQUEST:  return 0x01;
    case ActionType::LOGIN_RESPONSE: return 0x02;
    case ActionType::GAME_START:     return 0x0F;
    case ActionType::GAME_END:       return 0x10;
    case ActionType::ERROR:          return 0x12;
    case ActionType::UP_PRESS:       // Tous les PLAYER_INPUT
    case ActionType::UP_RELEASE:     // ...
      return 0x20;
    case ActionType::GAME_STATE:     return 0x21;
    case ActionType::AUTH:           return 0x22;
    case ActionType::BOSS_SPAWN:     return 0x23;
    case ActionType::BOSS_UPDATE:    return 0x24;
    case ActionType::ENEMY_HIT:      return 0x25;
    default:                         return 0xFF;
  }
}
```

### 3.3 Decoder - Serveur → Client

**Decoder.hpp** :
```cpp
class Decoder {
public:
  using DecodeFunc = std::function<Event(const std::vector<uint8_t>&)>;
  
  Decoder();
  void registerHandler(uint8_t packetType, DecodeFunc func);
  Event decode(const std::vector<uint8_t>& packet);
  
private:
  std::array<DecodeFunc, 256> handlers;
};
```

**DecodeFunc.cpp - Exemple de décodage** :
```cpp
Event DecodeLOGIN_RESPONSE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::LOGIN_RESPONSE;
  
  LOGIN_RESPONSE data;
  size_t offset = 2;  // Après type+flags
  
  uint32_t payloadLength = 0;
  if (!checkHeader(packet, offset, payloadLength)) 
    return Event{};
  
  data.success = packet[offset++];
  
  if (data.success == 1) {
    // Succès: récupère playerId et udpPort
    memcpy(&data.playerId, &packet[offset], sizeof(data.playerId));
    data.playerId = ntohs(data.playerId);
    offset += sizeof(data.playerId);
    
    memcpy(&data.udpPort, &packet[offset], sizeof(data.udpPort));
    data.udpPort = ntohs(data.udpPort);
  } else {
    // Échec: récupère errorCode et message
    memcpy(&data.errorCode, &packet[offset], sizeof(data.errorCode));
    data.errorCode = ntohs(data.errorCode);
    offset += sizeof(data.errorCode);
    
    uint8_t msgLen = packet[offset++];
    data.message = std::string(reinterpret_cast<const char*>(&packet[offset]), msgLen);
  }
  
  evt.data = data;
  return evt;
}
```

**Fonction utilitaire de vérification** :
```cpp
bool checkHeader(const std::vector<uint8_t>& packet, size_t& offset,
                 uint32_t& payloadLength) {
  memcpy(&payloadLength, &packet[offset], sizeof(payloadLength));
  payloadLength = ntohl(payloadLength);
  offset += 4;
  
  return payloadLength == packet.size() - 6;
}
```

### 3.4 Enregistrement des Handlers

**EncodeFunc.hpp** :
```cpp
inline void SetupEncoder(Encoder& encoder) {
  encoder.registerHandler(ActionType::AUTH, Auth);
  encoder.registerHandler(ActionType::UP_PRESS, PlayerInputFunc);
  encoder.registerHandler(ActionType::UP_RELEASE, PlayerInputFunc);
  // ... tous les handlers d'entrée
  encoder.registerHandler(ActionType::LOGIN_REQUEST, LoginRequestFunc);
  encoder.registerHandler(ActionType::LOGIN_RESPONSE, LoginResponseFunc);
  // ... autres handlers
}
```

**DecodeFunc.cpp** :
```cpp
void SetupDecoder(Decoder& decoder) {
  decoder.registerHandler(0x01, DecodeLOGIN_REQUEST);
  decoder.registerHandler(0x02, DecodeLOGIN_RESPONSE);
  decoder.registerHandler(0x0F, DecodeGAME_START);
  decoder.registerHandler(0x10, DecodeGAME_END);
  decoder.registerHandler(0x12, DecodeERROR);
  decoder.registerHandler(0x20, DecodePLAYER_INPUT);
  decoder.registerHandler(0x21, DecodeGAME_STATE);
  decoder.registerHandler(0x22, DecodeAUTH);
  decoder.registerHandler(0x23, DecodeBOSS_SPAWN);
  decoder.registerHandler(0x24, DecodeBOSS_UPDATE);
  decoder.registerHandler(0x25, DecodeENEMY_HIT);
}
```

---

## 4. Network Manager

### 4.1 Interface Publique

**NetworkManager.hpp** :
```cpp
class NetworkManager {
public:
  NetworkManager();
  ~NetworkManager();
  
  // Connexion/Déconnexion
  bool Connect(const std::string& ip, int port);
  void Disconnect();
  bool IsConnected() const { return tcpConnected; }
  
  // Communication
  void SendAction(Action action);
  Event PopEvent();
  
private:
  // État
  std::mutex mut;
  bool tcpConnected = false;
  bool udpConnected = false;
  bool running = false;
  uint16_t playerId = 0;
  int udpPort = -1;
  
  // Réseau (ASIO)
  asio::io_context ioContext;
  asio::ip::tcp::socket tcpSocket;
  asio::ip::udp::socket udpSocket;
  asio::ip::udp::endpoint udpEndpoint;
  
  // Threading
  std::thread networkThread;
  
  // Buffers circulaires
  CircularBuffer<Event> eventBuffer;
  CircularBuffer<Action> actionBuffer;
  
  // Composants
  Decoder decoder;
  Encoder encoder;
  std::vector<uint8_t> recvTcpBuffer;
  
  // Méthodes internes
  void ThreadLoop();
  int ConnectTCP();
  int ConnectUDP();
  void ReadTCP();
  void ReadUDP();
  void ProcessTCPRecvBuffer();
  void SendActionServer();
  Event DecodePacket(std::vector<uint8_t>& packet);
  void AuthAction();
  void SendUdp(std::vector<uint8_t>& packet);
  void SendTcp(std::vector<uint8_t>& packet);
};
```

### 4.2 Cycle de Vie

**Initialisation** :
```cpp
NetworkManager::NetworkManager()
    : eventBuffer(50), actionBuffer(50),
      tcpSocket(ioContext), udpSocket(ioContext) {
  SetupDecoder(decoder);
  SetupEncoder(encoder);
}
```

**Connexion** :
```cpp
bool NetworkManager::Connect(const std::string& ip, int port) {
  serverIP = ip;
  tcpPort = port;
  
  if (running) return false;
  
  running = true;
  networkThread = std::thread(&NetworkManager::ThreadLoop, this);
  return true;
}
```

**Boucle réseau principale** :
```cpp
void NetworkManager::ThreadLoop() {
  while (running) {
    // Connexion/reconnexion TCP
    if (!tcpConnected) {
      if (ConnectTCP() == -1) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        continue;
      }
    }
    
    // Connexion UDP si port disponible
    if (!udpConnected && udpPort != -1) {
      ConnectUDP();
    }
    
    // Lecture réseau
    if (tcpConnected) ReadTCP();
    if (udpConnected && udpPort != -1) ReadUDP();
    
    // Envoi des actions en attente
    SendActionServer();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
```

### 4.3 Lecture TCP et Reconstruction

```cpp
void NetworkManager::ReadTCP() {
  char tempBuffer[1024];
  asio::error_code error;
  
  size_t bytesReceived = tcpSocket.read_some(
    asio::buffer(tempBuffer, sizeof(tempBuffer)), error);
  
  if (!error && bytesReceived > 0) {
    recvTcpBuffer.insert(recvTcpBuffer.end(), 
                        tempBuffer, tempBuffer + bytesReceived);
    ProcessTCPRecvBuffer();
  } else if (error == asio::error::eof) {
    tcpConnected = false;  // Déconnexion
  }
  // ... gestion autres erreurs
}

void NetworkManager::ProcessTCPRecvBuffer() {
  while (recvTcpBuffer.size() >= 6) {
    // Lit la taille du payload
    uint32_t packetSize;
    memcpy(&packetSize, &recvTcpBuffer[2], sizeof(packetSize));
    packetSize = ntohl(packetSize);
    
    // Vérifie si paquet complet
    if (recvTcpBuffer.size() < 6 + packetSize) return;
    
    // Extrait le paquet
    std::vector<uint8_t> packet(recvTcpBuffer.begin(),
                                recvTcpBuffer.begin() + 6 + packetSize);
    recvTcpBuffer.erase(recvTcpBuffer.begin(),
                        recvTcpBuffer.begin() + 6 + packetSize);
    
    // Décodage
    Event evt = DecodePacket(packet);
    
    // Traitement spécial LOGIN_RESPONSE
    if (evt.type == EventType::LOGIN_RESPONSE) {
      const auto* resp = std::get_if<LOGIN_RESPONSE>(&evt.data);
      if (resp && resp->success == 1) {
        udpPort = resp->udpPort;     // Sauvegarde port UDP
        playerId = resp->playerId;   // Sauvegarde ID joueur
        AuthAction();                // Envoi AUTH UDP automatique
      }
    }
    
    // Ajout au buffer d'événements
    std::lock_guard<std::mutex> lock(mut);
    eventBuffer.push(evt);
  }
}
```

### 4.4 Envoi d'Actions

```cpp
void NetworkManager::SendAction(Action action) {
  std::lock_guard<std::mutex> lock(mut);
  actionBuffer.push(action);  // Ajout thread-safe au buffer
}

void NetworkManager::SendActionServer() {
  std::unique_lock<std::mutex> lock(mut);
  size_t bufferSize = actionBuffer.size();
  
  for (size_t i = 0; i < bufferSize; ++i) {
    auto opt = actionBuffer.peek();
    if (!opt.has_value()) break;
    
    const Action& action = opt.value();
    size_t protocol = UseUdp(action.type);
    
    bool sent = false;
    std::vector<uint8_t> packet;
    
    if ((protocol == 0 || protocol == 1) && udpConnected) {
      packet = encoder.encode(action, protocol);
      lock.unlock();
      SendUdp(packet);    // Envoi UDP
      lock.lock();
      sent = true;
    } else if (protocol == 2 && tcpConnected) {
      packet = encoder.encode(action, protocol);
      lock.unlock();
      SendTcp(packet);    // Envoi TCP
      lock.lock();
      sent = true;
    }
    
    if (sent) {
      actionBuffer.pop();  // Retire uniquement si envoyé
    } else {
      break;  // Arrête si échec d'envoi
    }
  }
}
```

### 4.5 Authentification UDP Automatique

```cpp
void NetworkManager::AuthAction() {
  Action ac;
  ac.type = ActionType::AUTH;
  
  AuthUDP authData;
  authData.playerId = playerId;  // ID reçu du serveur
  
  ac.data = authData;
  SendAction(ac);  // Envoi via le système normal
}
```

---

## 5. Buffer Circulaire Thread-Safe

**CircularBuffer.hpp** :
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
  explicit CircularBuffer(size_t capacity)
      : m_buffer(capacity), m_maxItem(capacity) {}
  
  void push(const T& item) {
    m_buffer[m_lastItemIndex] = item;
    m_lastItemIndex = (m_lastItemIndex + 1) % m_maxItem;
    
    if (m_nbItem < m_maxItem) {
      m_nbItem++;
    } else {
      m_firstItemIndex = (m_firstItemIndex + 1) % m_maxItem; // Écrase
    }
  }
  
  std::optional<T> pop() {
    if (m_nbItem == 0) return std::nullopt;
    
    T item = m_buffer[m_firstItemIndex];
    m_firstItemIndex = (m_firstItemIndex + 1) % m_maxItem;
    m_nbItem--;
    return item;
  }
  
  std::optional<T> peek() {
    if (m_nbItem == 0) return std::nullopt;
    return m_buffer[m_firstItemIndex];
  }
  
  size_t size() const { return m_nbItem; }
  bool isEmpty() const { return m_nbItem == 0; }
  bool isFull() const { return m_nbItem == m_maxItem; }
};
```

---

## 6. Utilisation du Client

### 6.1 Exemple Minimal

```cpp
#include "NetworkManager.hpp"
#include <iostream>

int main() {
  NetworkManager network;
  
  // Connexion
  if (!network.Connect("127.0.0.1", 8080)) {
    std::cerr << "Échec démarrage connexion" << std::endl;
    return 1;
  }
  
  // Attente connexion
  while (!network.IsConnected()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  
  // Envoi login
  Action loginAction;
  loginAction.type = ActionType::LOGIN_REQUEST;
  
  LoginReq loginData{"joueur1", "hash_mdp"};
  loginAction.data = loginData;
  network.SendAction(loginAction);
  
  // Boucle principale ...
  
    // Envoi entrées (exemple)
    Action inputAction;
    inputAction.type = ActionType::UP_PRESS;
    
    PlayerInput input{true, false, false, false, 0};
    inputAction.data = input;
    network.SendAction(inputAction);
  // Boucle principale Fin...

  
  network.Disconnect();
  return 0;
}
```

### 6.2 Points d'Attention

1. **Thread-safety** : `SendAction()` et `PopEvent()` sont thread-safe
2. **Buffer overflow** : Les buffers circulaires écrasent les anciennes données
3. **Reconnexion** : Automatique pour TCP, manuelle pour UDP

---

## 7. Extension du Système

### 7.1 Ajout d'un Nouveau Message - Guide en 5 Étapes

**Étape 1 : Définir dans Action.hpp**
```cpp
// 1. Ajouter à l'enum
enum class ActionType : uint8_t {
  // ...
  PLAYER_CHAT = 0x30,
};

// 2. Créer structure
struct PlayerChat {
  std::string message;
  uint8_t channel;
};

// 3. Ajouter au variant
using ActionData = std::variant<std::monostate,
  // ...
  PlayerChat>;  // AJOUT
```

**Étape 2 : Ajouter dans Event.hpp si réponse**
```cpp
enum class EventType : uint8_t {
  // ...
  CHAT_MESSAGE = 0x31,
};

struct CHAT_MESSAGE {
  uint16_t senderId;
  std::string message;
  uint8_t channel;
};

using EventData = std::variant<std::monostate,
  // ...
  CHAT_MESSAGE>;  // AJOUT
```

**Étape 3 : Implémenter encodage (EncodeFunc.hpp)**
```cpp
inline void PlayerChatFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* chat = std::get_if<PlayerChat>(&a.data);
  if (!chat) return;
  
  out.clear();
  out.push_back(chat->channel);
  //...
  out.push_back(msgLen);
}
```

**Étape 4 : Implémenter décodage (DecodeFunc.cpp)**
```cpp
Event DecodeCHAT_MESSAGE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::CHAT_MESSAGE;
  
  CHAT_MESSAGE data;
  //..
  evt.data = data;
  return evt;
}
```

**Étape 5 : Enregistrer et configurer**
```cpp
// EncodeFunc.hpp
inline void SetupEncoder(Encoder& encoder) {
  // ... existants
  encoder.registerHandler(ActionType::PLAYER_CHAT, PlayerChatFunc);
}

// DecodeFunc.cpp
void SetupDecoder(Decoder& decoder) {
  // ... existants
  decoder.registerHandler(0x31, DecodeCHAT_MESSAGE);
}

// Action.hpp
inline size_t UseUdp(ActionType type) {
  switch (type) {
    // ...
    case ActionType::PLAYER_CHAT:
      return 0;  // UDP
  }
}

// Encoder.cpp
inline uint8_t getType(const Action& a) {
  switch (a.type) {
    // ...
    case ActionType::PLAYER_CHAT:
      return 0x30;
  }
}
```

---

## 9. Points Clés de l'Architecture

1. **Découplage** : Encodeur/Décodeur indépendants du transport
2. **Thread-safety** : Buffers circulaires avec mutex
3. **Performance** : Sockets non-bloquants, buffers fixes
4. **Fiabilité** : TCP pour messages critiques, UDP pour temps-réel
5. **Automatisation** : Connexion UDP automatique après login
