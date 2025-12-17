# Documentation Développeur - Module Réseau

Cette documentation explique l'architecture du module réseau, les fonctions clés du gestionnaire de connexion et la procédure pour implémenter de nouveaux messages (Actions/Events).

## I. Architecture du Code

L'architecture est structurée autour de deux domaines principaux : la gestion des connexions (`Engine/Network`) et la sérialisation des données (`Shared` ou `network/`).

### A. Fichiers Partagés (`Shared`/`network/`) : Sérialisation

Ces fichiers gèrent la transformation des structures de données C++ en paquets binaires (encodage) et inversement (décodage).

| Fichier | Rôle |
| --- | --- |
| `Action.hpp` | Définit les structures de messages envoyés (`Action`), l'énumération `ActionType`, et le `std::variant` `ActionData` pour les payloads. |
| `Event.hpp` | Définit les structures des messages reçus (`Event`). |
| `Encoder.hpp`/`.cpp` | Classe de base pour la sérialisation. Mappe `ActionType` à sa fonction d'encodage. |
| `EncodeFunc.hpp` | Implémente les fonctions d'encodage spécifiques (ex: `LoginRequestFunc`). Contient `SetupEncoder`. |
| `Decoder.hpp`/`.cpp` | Classe de base pour la désérialisation. Mappe le code de paquet à sa fonction de décodage. |
| `DecodeFunc.hpp`/`.cpp` | Implémente les fonctions de décodage spécifiques (ex: `DecodeLOGIN_REQUEST`). Contient `SetupDecoder`. |

### B. Dossier `Engine/Network` : Connexion et Buffers

Ces fichiers gèrent l'établissement et le maintien de la connexion, ainsi que la gestion des files d'attente de messages.

| Fichier | Rôle |
| --- | --- |
| `NetworkManager.hpp`/`.cpp` | Gestionnaire central. Gère les sockets TCP/UDP (via *Asio*), le thread réseau (`ThreadLoop`), la connexion, la lecture et l'envoi. |
| `CircularBuffer.hpp` | Structure de données pour gérer les files d'attente d'Actions (à envoyer) et d'Events (à traiter) de manière thread-safe. |

## II. Fonctions Clés du NetworkManager

Le `NetworkManager` est l'interface principale entre le moteur de jeu et la couche réseau. Il utilise des tampons circulaires pour les communications thread-safe.

### 1. `bool Connect(const std::string& ip, int port)`

**Rôle :** Initialise et lance la boucle de communication dans un thread séparé.

```cpp
bool NetworkManager::Connect(const std::string& ip, int port) {
  serverIP = ip;
  tcpPort = port;
  if (running) {
    std::cerr << "Try connect to TCP Server with IP(" << serverIP << ")"
              << std::endl;
    return false;
  }
  running = true;
  networkThread = std::thread(&NetworkManager::ThreadLoop, this);
  return true;
}

```

**Explication :**
Cette fonction initialise l'état du manager et crée un `std::thread` qui exécute la méthode privée `ThreadLoop()`, isolant les opérations réseau du thread principal du jeu.

### 2. `void SendAction(Action action)`

**Rôle :** Place une `Action` (message à envoyer au serveur) dans la file d'attente d'envoi.

```cpp
void NetworkManager::SendAction(Action action) {
  std::lock_guard<std::mutex> lock(mut);
  actionBuffer.push(action);
}

```

**Explication :**
Cette API utilise un `std::lock_guard` pour garantir la sécurité thread lors de l'ajout de l'`Action` dans le `actionBuffer`. Le thread réseau traitera ce buffer plus tard via `SendActionServer()`.

### 3. `Event PopEvent()`

**Rôle :** Récupère le prochain `Event` (message reçu) de la file d'attente pour le traitement par le moteur de jeu.

```cpp
Event NetworkManager::PopEvent() {
  std::lock_guard<std::mutex> lock(mut);
  std::optional<Event> evt = eventBuffer.pop();
  if (evt.has_value()) {
    return evt.value();
  }
  return Event{};
}

```

**Explication :**
Cette API est appelée par le thread principal. Elle retire le premier `Event` du `eventBuffer` de manière thread-safe et retourne un `Event` valide ou un `Event{}` vide.

### 4. `void ThreadLoop()`

**Rôle :** La boucle d'exécution du thread réseau.

```cpp
void NetworkManager::ThreadLoop() {
  while (running) {
    if (!tcpConnected) {
      if (ConnectTCP() == -1) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
      }
    }
    // ... ConnectUDP logic ...
    if (tcpConnected) {
      ReadTCP();
    }
    if (udpConnected && udpPort != -1) {
      ReadUDP();
    }
    SendActionServer();

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ioContext.poll();
  }
}

```

**Explication :**
Cette boucle gère la persistance de la connexion. Elle tente de se connecter/reconnecter, appelle `ReadTCP()` et `ReadUDP()` pour lire les données, puis `SendActionServer()` pour envoyer les messages en attente.

### 5. `void SendActionServer()`

**Rôle :** Vide le `actionBuffer`, encode les Actions, et les envoie via le protocole approprié (TCP ou UDP).

```cpp
void NetworkManager::SendActionServer() {
  std::lock_guard<std::mutex> lock(mut);
  while (true) {
    std::optional<Action> opt = actionBuffer.peek();
    if (!opt.has_value()) break;

    const Action& action = opt.value();
    size_t protocol = UseUdp(action.type);

    bool sent = false;
    // ... encode action using encoder.encode() ...
    // ... call SendUdp() or SendTcp() based on protocol ...

    if (sent) {
      actionBuffer.pop();
    } else {
      break;
    }
  }
}

```

**Explication :**
Cette fonction itère sur le `actionBuffer`. Elle utilise la fonction `UseUdp(action.type)` pour déterminer le protocole, encode l'`Action` en paquet binaire (via `encoder.encode()`) et envoie le paquet. Si l'envoi réussit, l'`Action` est retirée du buffer.

## III. Implémentation d'un nouvel Action/Event

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
  //...

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
networkManager.SendAction(action);

```

#### Recevoir et Traiter un Event :

```cpp
// Moteur (dans la boucle principale)
Event event = networkManager.PopEvent();
if (event.type == EventType::NEW_MESSAGE) {
  if (const auto* data = std::get_if<NewMessageData>(&event.data)) {
    // Le message est prêt : traiter data->id et data->value
  }
}

```