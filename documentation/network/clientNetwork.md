# Documentation Développeur - Module Réseau

Cette documentation explique l'architecture du module réseau, les fonctions clés du gestionnaire de connexion et la procédure pour implémenter de nouveaux messages (Actions/Events).

## I. Architecture du Code

L'architecture est structurée autour de deux domaines principaux : la gestion des connexions (`EngineModule/src/subsystems/network`) et la sérialisation des données (`network/`).

### A. Fichiers Partagés (`network/`) : Sérialisation

Ces fichiers gèrent la transformation des structures de données C++ en paquets binaires (encodage) et inversement (décodage).

| Fichier | Rôle |
| --- | --- |
| `Action.hpp` | Définit les structures de messages envoyés (`Action`), l'énumération `ActionType`, et le `std::variant` `ActionData` pour les payloads. |
| `Event.hpp` | Définit les structures des messages reçus (`Event`). |
| `Encoder.hpp`/`.cpp` | Classe de base pour la sérialisation. Mappe `ActionType` à sa fonction d'encodage. |
| `EncodeFunc.hpp` | Implémente les fonctions d'encodage spécifiques (ex: `LoginRequestFunc`). Contient `SetupEncoder`. |
| `Decoder.hpp`/`.cpp` | Classe de base pour la désérialisation. Mappe le code de paquet à sa fonction de décodage. |
| `DecodeFunc.hpp`/`.cpp` | Implémente les fonctions de décodage spécifiques (ex: `DecodeLOGIN_REQUEST`). Contient `SetupDecoder`. |

### B. Dossier `EngineModule/src/subsystems/network` : Connexion et Buffers

Ces fichiers gèrent l'établissement et le maintien de la connexion, ainsi que la gestion des files d'attente de messages.

| Fichier | Rôle |
| --- | --- |
| `NetworkSubsystem.hpp`/`.cpp` | Gestionnaire central implémentant `ISubsystem`. Gère les sockets TCP/UDP (via *Asio*), le thread réseau (`ThreadLoop`), la connexion, la lecture et l'envoi. |
| `CircularBuffer.hpp` | Structure de données pour gérer les files d'attente d'Actions (à envoyer) et d'Events (à traiter) de manière thread-safe. |
| `network_export.hpp` | Définit les macros d'exportation DLL pour la compatibilité Windows/Linux. |

## II. Fonctions Clés du NetworkSubsystem

Le `NetworkSubsystem` est l'interface principale entre le moteur de jeu et la couche réseau. Il implémente l'interface `ISubsystem` et utilise des tampons circulaires pour les communications thread-safe.

### 1. `bool Initialize()`

**Rôle :** Initialise le subsystem réseau en configurant les encodeurs et décodeurs.

```cpp
bool NetworkSubsystem::Initialize() {
  SetupDecoder(decoder);
  SetupEncoder(encoder);
  return true;
}
```

**Explication :**
Cette fonction est appelée lors du chargement du subsystem. Elle configure les tables de correspondance entre les types de messages et leurs fonctions d'encodage/décodage respectives.

### 2. `bool Connect(const std::string& ip, int port)`

**Rôle :** Initialise et lance la boucle de communication dans un thread séparé.

```cpp
bool NetworkSubsystem::Connect(const std::string& ip, int port) {
  serverIP = ip;
  tcpPort = port;
  if (running) {
    std::cerr << "Try connect to TCP Server with IP(" << serverIP << ")"
              << std::endl;
    return false;
  }
  running = true;
  networkThread = std::thread(&NetworkSubsystem::ThreadLoop, this);
  return true;
}
```

**Explication :**
Cette fonction initialise l'état du subsystem et crée un `std::thread` qui exécute la méthode privée `ThreadLoop()`, isolant les opérations réseau du thread principal du jeu.

### 3. `void Disconnect()`

**Rôle :** Ferme proprement toutes les connexions et arrête le thread réseau.

```cpp
void NetworkSubsystem::Disconnect() {
  Action disconnect;
  ClientLeave l;
  disconnect.type = ActionType::CLIENT_LEAVE;
  l.playerId = 0;
  disconnect.data = l;

  std::vector<uint8_t> packet = encoder.encode(disconnect, 2, 0, 0, 0);
  SendTcp(packet);

  running = false;

  asio::error_code ec;
  if (tcpSocket.is_open()) tcpSocket.close(ec);
  if (udpSocket.is_open()) udpSocket.close(ec);

  tcpConnected = false;
  udpConnected = false;
  tcpPort = -1;
  udpPort = -1;
  if (networkThread.joinable()) {
    networkThread.join();
  }
}
```

**Explication :**
Cette fonction envoie un message de déconnexion au serveur, arrête la boucle réseau, ferme les sockets TCP/UDP et attend que le thread réseau se termine proprement.

### 4. `void SendAction(Action action)`

**Rôle :** Place une `Action` (message à envoyer au serveur) dans la file d'attente d'envoi.

```cpp
void NetworkSubsystem::SendAction(Action action) {
  std::lock_guard<std::mutex> lock(mut);
  actionBuffer.push(action);
}
```

**Explication :**
Cette API utilise un `std::lock_guard` pour garantir la sécurité thread lors de l'ajout de l'`Action` dans le `actionBuffer`. Le thread réseau traitera ce buffer plus tard via `SendActionServer()`.

### 5. `Event PopEvent()`

**Rôle :** Récupère le prochain `Event` (message reçu) de la file d'attente pour le traitement par le moteur de jeu.

```cpp
Event NetworkSubsystem::PopEvent() {
  std::lock_guard<std::mutex> lock(mut);
  auto res = eventBuffer.pop();
  if (res.has_value()) return res.value();
  return Event{EventType::UNKNOWN};
}
```

**Explication :**
Cette API est appelée par le thread principal. Elle retire le premier `Event` du `eventBuffer` de manière thread-safe et retourne un `Event` valide ou un `Event{EventType::UNKNOWN}` vide.

### 6. `void ThreadLoop()`

**Rôle :** La boucle d'exécution du thread réseau.

```cpp
void NetworkSubsystem::ThreadLoop() {
  while (running) {
    if (!tcpConnected) {
      if (ConnectTCP() == -1) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::cout << "[TCP] try to reconnect in 10 seconds\n";
      }
    }
    if (!udpConnected && udpPort != -1) {
      ConnectUDP();
    }
    if (tcpConnected) {
      ReadTCP();
    }
    if (udpConnected && udpPort != -1) {
      ReadUDP();
      HandleRetransmission();
    }
    SendActionServer();

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
```

**Explication :**
Cette boucle gère la persistance de la connexion. Elle tente de se connecter/reconnecter, appelle `ReadTCP()` et `ReadUDP()` pour lire les données, `HandleRetransmission()` pour gérer les renvois de paquets UDP fiables, puis `SendActionServer()` pour envoyer les messages en attente.

### 7. `int ConnectTCP()` et `int ConnectUDP()`

**Rôle :** Établit les connexions TCP et UDP avec le serveur.

```cpp
int NetworkSubsystem::ConnectTCP() {
  try {
    asio::ip::tcp::endpoint endpoint(asio::ip::make_address(serverIP), tcpPort);

    asio::error_code error;
    tcpSocket.connect(endpoint, error);

    if (error) {
      std::cerr << "Connexion server TCP error: " << error.message() << "\n";
      return -1;
    }

    tcpSocket.non_blocking(true, error);
    if (error) {
      std::cerr << "Failed to set TCP socket non-blocking: " << error.message()
                << "\n";
      tcpSocket.close();
      return -1;
    }

    tcpConnected = true;
    std::cout << "Connected to TCP server!\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "TCP connection exception: " << error.what() << "\n";
    return -1;
  }
}
```

**Explication :**
Ces fonctions établissent les connexions réseau en mode non-bloquant. Le TCP est connecté en premier, puis le serveur communique le port UDP à utiliser via un message `LOGIN_RESPONSE`.

### 8. `void ReadTCP()` et `void ReadUDP()`

**Rôle :** Lisent les données depuis les sockets et les placent dans les buffers de réception.

```cpp
void NetworkSubsystem::ReadTCP() {
  char tempBuffer[1024];

  asio::error_code error;
  size_t bytesReceived =
      tcpSocket.read_some(asio::buffer(tempBuffer, sizeof(tempBuffer)), error);

  if (!error && bytesReceived > 0) {
    recvTcpBuffer.insert(recvTcpBuffer.end(), tempBuffer,
                         tempBuffer + bytesReceived);
    ProcessTCPRecvBuffer();
  } else if (error == asio::error::eof) {
    std::cout << "Serveur TCP déconnecté" << std::endl;
    tcpConnected = false;
  } else if (error == asio::error::would_block ||
             error == asio::error::try_again) {
    return;
  } else if (error) {
    std::cerr << "TCP read error: " << error.message() << std::endl;
    tcpConnected = false;
  }
}
```

**Explication :**
Ces fonctions lisent les données disponibles sur les sockets de manière non-bloquante. Pour TCP, les données sont accumulées dans `recvTcpBuffer` puis traitées par `ProcessTCPRecvBuffer()` qui gère le découpage des paquets. Pour UDP, les paquets sont directement décodés et ajoutés au buffer d'événements avec gestion des numéros de séquence pour détecter les duplicatas.

### 9. `void ProcessTCPRecvBuffer()`

**Rôle :** Traite le buffer TCP pour extraire et décoder les paquets complets.

```cpp
void NetworkSubsystem::ProcessTCPRecvBuffer() {
  while (recvTcpBuffer.size() >= 6) {
    uint32_t packetSize;
    memcpy(&packetSize, &recvTcpBuffer[2], sizeof(packetSize));
    packetSize = ntohl(packetSize);

    if (recvTcpBuffer.size() < 6 + packetSize) {
      return;
    }

    std::vector<uint8_t> packet(recvTcpBuffer.begin(),
                                recvTcpBuffer.begin() + 6 + packetSize);

    recvTcpBuffer.erase(recvTcpBuffer.begin(),
                        recvTcpBuffer.begin() + 6 + packetSize);

    Event evt = DecodePacket(packet);

    if (evt.type == EventType::LOGIN_RESPONSE) {
      const auto* input = std::get_if<LOGIN_RESPONSE>(&evt.data);
      if (!input) return;
      if (input->success == 1) {
        udpPort = input->udpPort;
        playerId = input->playerId;
        AuthAction();
      }
    }

    std::lock_guard<std::mutex> lock(mut);
    eventBuffer.push(evt);
  }
}
```

**Explication :**
TCP est un protocole orienté flux, les données peuvent arriver fragmentées. Cette fonction reconstruit les paquets complets en vérifiant d'abord qu'il y a assez de données pour former un paquet complet (header de 6 octets + payload). Elle gère également la réponse de login pour initialiser la connexion UDP.

### 10. `void SendActionServer()`

**Rôle :** Vide le `actionBuffer`, encode les Actions, et les envoie via le protocole approprié (TCP ou UDP).

```cpp
void NetworkSubsystem::SendActionServer() {
  while (true) {
    Action action;
    bool hasAction = false;

    {
      std::lock_guard<std::mutex> lock(mut);
      if (auto opt = actionBuffer.peek()) {
        action = opt.value();
        hasAction = true;
      }
    }

    if (!hasAction) break;

    size_t protocol = UseUdp(action.type);
    bool sent = false;
    if ((protocol == 0 || protocol == 1) && udpConnected) {
      uint16_t s, a;
      uint32_t ab;
      {
        std::lock_guard<std::mutex> lock(mut);
        s = ++seqNum;
        a = ack;
        ab = ack_bits;
      }
      std::vector<uint8_t> packet = encoder.encode(action, protocol, s, a, ab);
      if (protocol == 1) {
        std::lock_guard<std::mutex> lock(mut);
        clientHistory[s] = {s, packet, std::chrono::steady_clock::now(), 0};
      }
      SendUdp(packet);
      sent = true;
    } else if (protocol == 2 && tcpConnected) {
      std::vector<uint8_t> packet = encoder.encode(action, protocol, 0, 0, 0);
      SendTcp(packet);
      sent = true;
    }

    if (sent) {
      std::lock_guard<std::mutex> lock(mut);
      actionBuffer.pop();
    } else {
      break;
    }
  }
}
```

**Explication :**
Cette fonction itère sur le `actionBuffer`. Elle utilise la fonction `UseUdp(action.type)` pour déterminer le protocole (0 = UDP non fiable, 1 = UDP fiable, 2 = TCP), encode l'`Action` en paquet binaire (via `encoder.encode()`) avec les numéros de séquence et d'acquittement, et envoie le paquet. Pour les paquets UDP fiables (protocol == 1), elle les stocke dans `clientHistory` pour gérer les retransmissions. Si l'envoi réussit, l'`Action` est retirée du buffer.

### 11. `void HandleRetransmission()`

**Rôle :** Gère la retransmission des paquets UDP fiables non acquittés.

```cpp
void NetworkSubsystem::HandleRetransmission() {
  std::lock_guard<std::mutex> lock(mut);
  auto now = std::chrono::steady_clock::now();

  for (auto it = clientHistory.begin(); it != clientHistory.end();) {
    if (now - it->second.lastSent > std::chrono::milliseconds(100)) {
      if (it->second.retryCount < 15) {
        SendUdp(it->second.data);
        it->second.lastSent = now;
        it->second.retryCount++;
        ++it;
      } else {
        it = clientHistory.erase(it);
      }
    } else {
      ++it;
    }
  }
}
```

**Explication :**
Cette fonction implémente un mécanisme de fiabilité sur UDP. Elle parcourt l'historique des paquets envoyés et retransmet ceux qui n'ont pas été acquittés après 100ms. Après 15 tentatives, le paquet est abandonné. Les paquets sont retirés de l'historique lorsqu'ils sont acquittés par le serveur (voir la logique dans `ReadUDP()`).

## III. Gestion de la Fiabilité UDP

Le système implémente un mécanisme de fiabilité optionnelle pour UDP basé sur des numéros de séquence et des acquittements.

### Numéros de Séquence

Chaque paquet UDP envoyé reçoit un numéro de séquence unique (`seqNum`) qui est incrémenté à chaque envoi. Le paquet contient également le dernier numéro de séquence reçu (`ack`) et un champ de bits (`ack_bits`) indiquant quels paquets précédents ont été reçus.

### Détection des Duplicatas

Lors de la réception d'un paquet UDP, le système vérifie si le paquet a déjà été traité en comparant son numéro de séquence avec l'historique récent :

```cpp
void NetworkSubsystem::ReadUDP() {
  // ... lecture du paquet ...
  
  std::lock_guard<std::mutex> lock(mut);
  uint16_t newSeq = evt.seqNum;
  bool isDuplicate = false;

  if (newSeq == ack) {
    isDuplicate = true;
  } else if (static_cast<uint16_t>(newSeq - ack) < 32768) {
    uint16_t shift = newSeq - ack;
    if (shift < 32) {
      ack_bits <<= shift;
      ack_bits |= (1 << (shift - 1));
    } else {
      ack_bits = 0;
    }
    ack = newSeq;
  } else {
    uint16_t shift = ack - newSeq;
    if (shift > 0 && shift <= 32) {
      if (ack_bits & (1 << (shift - 1))) {
        isDuplicate = true;
      }
      ack_bits |= (1 << (shift - 1));
    } else {
      isDuplicate = true;
    }
  }
  
  // Traiter les acquittements reçus
  clientHistory.erase(evt.ack);
  for (int i = 0; i < 32; ++i) {
    if (evt.ack_bits & (1 << i)) {
      clientHistory.erase(static_cast<uint16_t>(evt.ack - (i + 1)));
    }
  }
  
  if (!isDuplicate) eventBuffer.push(evt);
}
```

### Protocoles Disponibles

- **Protocol 0** : UDP non fiable - Pas de retransmission, utilisé pour les données fréquentes et non critiques (positions de joueurs)
- **Protocol 1** : UDP fiable - Avec retransmission, utilisé pour les actions importantes mais sensibles à la latence
- **Protocol 2** : TCP - Fiable et ordonné, utilisé pour les données critiques (login, données de jeu importantes)

## IV. CircularBuffer : File d'Attente Thread-Safe

Le `CircularBuffer` est une structure de données FIFO (First In, First Out) implémentée comme un buffer circulaire pour optimiser les performances.

### Caractéristiques

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

  void push(const T &item);
  std::optional<T> pop();
  std::optional<T> peek();
  bool isEmpty();
  bool isFull();
  size_t size();
};
```

### Opérations

- **push()** : Ajoute un élément. Si le buffer est plein, écrase le plus ancien
- **pop()** : Retire et retourne le plus ancien élément
- **peek()** : Retourne le plus ancien élément sans le retirer
- **isEmpty()** : Vérifie si le buffer est vide
- **isFull()** : Vérifie si le buffer est plein

### Utilisation avec Mutex

Le `CircularBuffer` n'est pas thread-safe par lui-même. Le `NetworkSubsystem` utilise un `std::mutex` pour protéger les accès concurrents :

```cpp
void NetworkSubsystem::SendAction(Action action) {
  std::lock_guard<std::mutex> lock(mut);
  actionBuffer.push(action);
}

Event NetworkSubsystem::PopEvent() {
  std::lock_guard<std::mutex> lock(mut);
  auto res = eventBuffer.pop();
  if (res.has_value()) return res.value();
  return Event{EventType::UNKNOWN};
}
```

## V. Implémentation d'un nouvel Action/Event

Prenons l'exemple d'un nouveau message, `ActionType::NEW_MESSAGE`, avec une structure `NewMessageData`.

### Étape 1: Définition du Message (Fichiers `Action.hpp` / `Event.hpp`)

#### Ajouter le Type :

```cpp
// Action.hpp (ou Event.hpp)
enum class ActionType : uint8_t {
  // ... types existants
  NEW_MESSAGE, // Nouveau type
  // ...
};
```

#### Définir la Structure de Données :

```cpp
// Action.hpp (ou Event.hpp)
struct NewMessageData {
  uint16_t id;
  float value;
};
```

#### Mettre à Jour le Variant (Payload) :

```cpp
// Action.hpp (ou Event.hpp)
using ActionData = std::variant<
  // ... structures existantes
  NewMessageData // Ajouter la nouvelle structure
>;
```

#### Définir le protocole :

```cpp
// Action.hpp
inline size_t UseUdp(ActionType type) {
  switch (type) {
    // ... types existants
    case ActionType::NEW_MESSAGE:
      return 2; // TCP (ou 0/1 pour UDP)
    default:
      return 2;
  }
}
```

### Étape 2: Implémentation de la Sérialisation (Action -> Paquet Binaire)

#### Créer la fonction d'encodage (dans `EncodeFunc.hpp`) :

```cpp
// EncodeFunc.hpp
inline void NewMessageFunc(const Action& a, std::vector<uint8_t>& out) {
  const auto* data = std::get_if<NewMessageData>(&a.data);
  if (!data) return;
  
  // Exemple de sérialisation d'un uint16_t et d'un float
  uint16_t netId = htons(data->id);
  out.insert(out.end(), (uint8_t*)&netId, (uint8_t*)&netId + sizeof(netId));
  
  uint8_t floatBytes[sizeof(float)];
  htonf(data->value, floatBytes); // Utiliser la fonction d'aide htonf
  out.insert(out.end(), floatBytes, floatBytes + sizeof(float));
}
```

#### Enregistrer l'handler (dans `EncodeFunc.hpp: SetupEncoder`) :

```cpp
// EncodeFunc.hpp
inline void SetupEncoder(Encoder& encoder) {
  // ... handlers existants
  encoder.registerHandler(ActionType::NEW_MESSAGE, NewMessageFunc);
}
```

#### Définir le code du paquet (dans `Encode.cpp: getType`) :

```cpp
// Encode.cpp
inline uint8_t getType(const Action& a) {
  switch (a.type) {
    // ... types existants
    case ActionType::NEW_MESSAGE:
      return 0x28; // Choisir un code unique, ex: 0x28
    // ...
    default:
      return 0xFF;
  }
}
```

### Étape 3: Implémentation de la Désérialisation (Paquet Binaire -> Event)

#### Créer la fonction de décodage (dans `DecodeFunc.hpp` et `DecodeFunc.cpp`) :

```cpp
// DecodeFunc.cpp
Event DecodeNEW_MESSAGE(const std::vector<uint8_t>& packet) {
  Event evt;
  evt.type = EventType::NEW_MESSAGE;
  NewMessageData data;
  size_t offset = 2; // Après le type de paquet

  // ... logique de vérification du header (checkHeader) ...

  // Désérialisation d'un uint16_t
  memcpy(&data.id, &packet[offset], sizeof(data.id));
  data.id = ntohs(data.id);
  offset += sizeof(data.id);
  
  // Désérialisation d'un float
  uint8_t floatBytes[sizeof(float)];
  memcpy(floatBytes, &packet[offset], sizeof(float));
  ntohf(floatBytes, &data.value);
  offset += sizeof(float);

  evt.data = data;
  return evt;
}
```

#### Enregistrer l'handler (dans `DecodeFunc.cpp: SetupDecoder`) :

```cpp
// DecodeFunc.cpp
void SetupDecoder(Decoder& decoder) {
  // ... handlers existants
  decoder.registerHandler(0x28, DecodeNEW_MESSAGE); // Utiliser le même code 0x28
}
```

### Étape 4: Utilisation dans le Moteur

#### Envoyer une Action :

```cpp
// Moteur
NewMessageData data = {123, 45.6f};
Action action;
action.type = ActionType::NEW_MESSAGE;
action.data = data;
networkSubsystem->SendAction(action);
```

#### Recevoir et Traiter un Event :

```cpp
// Moteur (dans la boucle principale)
Event event = networkSubsystem->PopEvent();
if (event.type == EventType::NEW_MESSAGE) {
  if (const auto* data = std::get_if<NewMessageData>(&event.data)) {
    // Le message est prêt : traiter data->id et data->value
    std::cout << "Received NEW_MESSAGE: id=" << data->id 
              << ", value=" << data->value << std::endl;
  }
}
```

## VI. Exportation DLL (Windows/Linux)

Le fichier `network_export.hpp` définit les macros nécessaires pour l'exportation de symboles lors de la compilation en tant que DLL/bibliothèque partagée.

### Configuration

```cpp
#pragma once

#ifdef _WIN32
    #ifdef NETWORK_EXPORTS
        #define NETWORK_API __declspec(dllexport)
    #else
        #define NETWORK_API __declspec(dllimport)
    #endif
#else
    #define NETWORK_API
#endif
```

### Utilisation

La macro `NETWORK_API` doit être utilisée avant la déclaration de la classe :

```cpp
class NETWORK_API NetworkSubsystem : public ISubsystem {
  // ...
};
```

### Fonction de Création du Subsystem

Le subsystem fournit une fonction de création qui permet au moteur de l'instancier dynamiquement :

```cpp
#ifdef _WIN32
__declspec(dllexport) ISubsystem* CreateSubsystem() {
    return new NetworkSubsystem();
}
#else
extern "C" {
ISubsystem* CreateSubsystem() { return new NetworkSubsystem(); }
}
#endif
```

## VII. Bonnes Pratiques et Recommandations

### Gestion de la Mémoire

- Toujours vérifier les `std::optional` retournés par les buffers circulaires
- Utiliser `std::get_if` pour accéder aux données du variant de manière sûre
- Libérer les ressources dans `Shutdown()` et le destructeur

### Thread Safety

- Toujours protéger les accès aux buffers partagés avec des mutex
- Éviter de maintenir les locks trop longtemps
- Utiliser `std::lock_guard` pour garantir la libération automatique des locks

### Gestion des Erreurs

- Vérifier les codes d'erreur d'Asio
- Gérer proprement les déconnexions et tentatives de reconnexion
- Logger les erreurs pour faciliter le débogage

### Performance

- Minimiser la taille des paquets en utilisant des types appropriés
- Utiliser UDP pour les données fréquentes et non critiques
- Regrouper plusieurs petites actions en un seul paquet si possible

### Débogage

- Utiliser les logs de débogage existants dans `ProcessTCPRecvBuffer()` et ailleurs
- Ajouter des logs pour les nouveaux types de messages
- Vérifier les numéros de séquence et acquittements lors de problèmes de fiabilité UDP

## VIII. Diagramme de Flux

### Envoi d'une Action

1. Le moteur appelle `SendAction(action)`
2. L'action est ajoutée à `actionBuffer` (thread-safe)
3. `ThreadLoop()` appelle `SendActionServer()`
4. L'action est encodée via `encoder.encode()`
5. Le paquet est envoyé via TCP ou UDP selon le protocole
6. Pour UDP fiable, le paquet est stocké dans `clientHistory`
7. L'action est retirée du buffer

### Réception d'un Event

1. `ThreadLoop()` appelle `ReadTCP()` ou `ReadUDP()`
2. Les données sont lues depuis le socket
3. Pour TCP : accumulation dans `recvTcpBuffer` puis `ProcessTCPRecvBuffer()`
4. Pour UDP : décodage immédiat avec vérification des duplicatas
5. L'event est décodé via `decoder.decode()`
6. L'event est ajouté à `eventBuffer` (thread-safe)
7. Le moteur appelle `PopEvent()` pour récupérer l'event
8. Le moteur traite l'event selon son type

### Cycle de Connexion

1. `Connect(ip, port)` démarre le thread réseau
2. `ThreadLoop()` appelle `ConnectTCP()`
3. Une fois connecté, envoi d'un message de login
4. Réception de `LOGIN_RESPONSE` avec le port UDP
5. `ConnectUDP()` établit la connexion UDP
6. Envoi de `AUTH` pour authentifier la connexion UDP
7. Le système est prêt pour l'échange de messages
8. `Disconnect()` ferme proprement toutes les connexions