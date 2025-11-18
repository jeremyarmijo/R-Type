# Documentation du Protocole Réseau R-Type

## Sommaire

1. [Choix TCP / UDP](#choix-tcp--udp)
2. [Structure Commune des Messages](#structure-commune-des-messages)
   - [Header (6 bytes)](#header-6-bytes)
3. [Messages TCP](#messages-tcp)
   - [0x01 - CLIENT_CONNECT](#0x01---client_connect)
   - [0x02 - SERVER_WELCOME](#0x02---server_welcome)
   - [0x03 - LOBBY_LIST](#0x03---lobby_list)
   - [0x04 - LOBBY_JOIN](#0x04---lobby_join)
   - [0x05 - LOBBY_JOINED](#0x05---lobby_joined)
   - [0x06 - CHAT_MESSAGE](#0x06---chat_message)
   - [0x07 - GAME_START](#0x07---game_start)
   - [0x08 - GAME_END](#0x08---game_end)
   - [0x09 - PLAYER_DISCONNECT](#0x09---player_disconnect)
   - [0x0A - ERROR](#0x0a---error)
4. [Messages UDP (Temps Réel)](#messages-udp-temps-réel)
   - [0x10 - PLAYER_INPUT](#0x10---player_input)
   - [0x11 - GAME_STATE](#0x11---game_state)
   - [0x12 - ENTITY_SPAWN](#0x12---entity_spawn)
   - [0x13 - ENTITY_DESTROY](#0x13---entity_destroy)
   - [0x14 - PLAYER_HIT](#0x14---player_hit)
   - [0x15 - POWERUP_COLLECTED](#0x15---powerup_collected)
   - [0x16 - FORCE_UPDATE](#0x16---force_update)
   - [0x17 - BOSS_SPAWN](#0x17---boss_spawn)
   - [0x18 - BOSS_UPDATE](#0x18---boss_update)
   - [0x19 - SCROLLING_UPDATE](#0x19---scrolling_update)
   - [0x1A - ACK](#0x1a---ack)
5. [Gestion de la Fiabilité UDP](#gestion-de-la-fiabilité-udp)
6. [Résumé des Types de Messages](#résumé-des-types-de-messages)

---

### Choix TCP / UDP

- **TCP** : Utilisé pour les communications critiques nécessitant une livraison garantie (connexion, chat, événements de lobby)
- **UDP** : Utilisé pour toutes les communications temps réel du gameplay (mouvements, tirs, états des entités)

Oui, tu dois rajouter le flag pour ACK dans cette section. Voici la version corrigée :

---

## Structure Commune des Messages

### Header (6 bytes)

Tous les messages, TCP et UDP, commencent par ce header :

| Offset | Champ    | Type   | Taille | Description |
|--------|----------|--------|--------|-------------|
| 0x00   | type     | uint8  | 1 byte | Identifiant du type de message |
| 0x01   | flags    | uint8  | 1 byte | Indicateurs de protocole et options |
| 0x02   | length   | uint32 | 4 bytes| Longueur du payload (big-endian) |

#### Flags disponibles

| Bit | Valeur | Signification |
|-----|--------|---------------|
| 0   | 0x01   | TCP protocol |
| 1   | 0x02   | UDP protocol |
| 3   | 0x08   | Requires ACK (UDP) |

**Note** : Le flag `Requires ACK` (0x08) est utilisé uniquement avec les messages UDP critiques qui nécessitent une confirmation de réception via un message ACK (0x1A).

---

## Messages TCP

### 0x01 - CLIENT_CONNECT

Envoyé par le client pour initier une connexion au serveur.

**Direction** : Client → Serveur

| Offset | Champ           | Type   | Taille | Description |
|--------|-----------------|--------|--------|-------------|
| 0x00   | playerNameLen   | uint8  | 1      | Longueur du pseudo (max 32) |
| 0x01   | playerName      | char[] | N      | Pseudo en UTF-8 |

**Exemple** : Connexion du joueur "Falcon"
```
Header: 01 01 07 00 00 00
Payload: 06 46 61 6C 63 6F 6E
```

---

### 0x02 - SERVER_WELCOME

Confirmation de connexion envoyée par le serveur.

**Direction** : Serveur → Client

| Offset | Champ      | Type   | Taille | Description |
|--------|------------|--------|--------|-------------|
| 0x00   | playerId   | uint16 | 2      | Identifiant unique attribué |
| 0x02   | serverTick | uint32 | 4      | Tick actuel du serveur |
| 0x06   | maxPlayers | uint8  | 1      | Nombre max de joueurs |
| 0x07   | udpPort    | uint16 | 2      | Port UDP à utiliser pour le gameplay |

**Exemple**
```
Header: 02 01 09 00 00 00
Payload: 00 01 00 00 03 E8 04 1F 90
```

---

### 0x03 - LOBBY_LIST

Liste des parties disponibles.

**Direction** : Serveur → Client

| Offset | Champ      | Type   | Taille | Description |
|--------|------------|--------|--------|-------------|
| 0x00   | lobbyCount | uint8  | 1      | Nombre de lobbies |
| 0x01   | lobbies[]  | struct | N×13   | Informations par lobby |

**Structure Lobby** (13 bytes) :

| Offset | Champ        | Type   | Taille | Description |
|--------|--------------|--------|--------|-------------|
| 0x00   | lobbyId      | uint16 | 2      | ID du lobby |
| 0x02   | nameLen      | uint8  | 1      | Longueur du nom |
| 0x03   | name         | char[] | 8      | Nom (padding avec 0x00) |
| 0x0B   | playerCount  | uint8  | 1      | Joueurs actuels |
| 0x0C   | maxPlayers   | uint8  | 1      | Capacité maximale |

---

### 0x04 - LOBBY_JOIN

Demande de rejoindre un lobby.

**Direction** : Client → Serveur

| Offset | Champ   | Type   | Taille | Description |
|--------|---------|--------|--------|-------------|
| 0x00   | lobbyId | uint16 | 2      | ID du lobby à rejoindre |

---

### 0x05 - LOBBY_JOINED

Confirmation d'entrée dans un lobby.

**Direction** : Serveur → Client

| Offset | Champ        | Type   | Taille | Description |
|--------|--------------|--------|--------|-------------|
| 0x00   | lobbyId      | uint16 | 2      | ID du lobby |
| 0x02   | playerCount  | uint8  | 1      | Nombre de joueurs |
| 0x03   | players[]    | struct | N×34   | Infos des joueurs présents |

**Structure Player Info** (34 bytes) :

| Offset | Champ      | Type   | Taille | Description |
|--------|------------|--------|--------|-------------|
| 0x00   | playerId   | uint16 | 2      | ID du joueur |
| 0x02   | nameLen    | uint8  | 1      | Longueur du pseudo |
| 0x03   | name       | char[] | 31     | Pseudo (padding avec 0x00) |

---

### 0x06 - CHAT_MESSAGE

Message textuel dans le lobby ou en jeu.

**Direction** : Bidirectionnelle

| Offset | Champ      | Type   | Taille | Description |
|--------|------------|--------|--------|-------------|
| 0x00   | senderId   | uint16 | 2      | ID de l'émetteur (0 = serveur) |
| 0x02   | messageLen | uint16 | 2      | Longueur du message (max 256) |
| 0x04   | message    | char[] | N      | Contenu UTF-8 |

---

### 0x07 - GAME_START

Notification du démarrage d'une partie.

**Direction** : Serveur → Clients

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | mapId       | uint16 | 2      | ID de la map/niveau |
| 0x02   | gameMode    | uint8  | 1      | Mode de jeu (0=coop, 1=versus) |
| 0x03   | difficulty  | uint8  | 1      | Difficulté (0-3) |
| 0x04   | startTick   | uint32 | 4      | Tick de démarrage |

---

### 0x08 - GAME_END

Notification de fin de partie avec résultats.

**Direction** : Serveur → Clients

| Offset | Champ        | Type   | Taille | Description |
|--------|--------------|--------|--------|-------------|
| 0x00   | victory      | uint8  | 1      | 1=victoire, 0=défaite |
| 0x01   | playerCount  | uint8  | 1      | Nombre de joueurs |
| 0x02   | scores[]     | struct | N×8    | Scores finaux |

**Structure Score** (8 bytes) :

| Offset | Champ    | Type   | Taille | Description |
|--------|----------|--------|--------|-------------|
| 0x00   | playerId | uint16 | 2      | ID du joueur |
| 0x02   | score    | uint32 | 4      | Score final |
| 0x06   | kills    | uint16 | 2      | Nombre d'ennemis tués |

---

### 0x09 - PLAYER_DISCONNECT

Notification de déconnexion d'un joueur.

**Direction** : Serveur → Clients

| Offset | Champ    | Type   | Taille | Description |
|--------|----------|--------|--------|-------------|
| 0x00   | playerId | uint16 | 2      | ID du joueur déconnecté |
| 0x02   | reason   | uint8  | 1      | Raison (0=normal, 1=timeout, 2=kick) |

---

### 0x0A - ERROR

Message d'erreur du serveur.

**Direction** : Serveur → Client

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | errorCode   | uint16 | 2      | Code d'erreur |
| 0x02   | messageLen  | uint8  | 1      | Longueur du message |
| 0x03   | message     | char[] | N      | Description de l'erreur |

**Codes d'erreur** :
- `0x0001` : Lobby plein
- `0x0002` : Pseudo déjà utilisé
- `0x0003` : Version du protocole incompatible
- `0x0004` : Serveur plein
- `0x0005` : Lobby inexistant

---

## Messages UDP (Temps Réel)

### 0x10 - PLAYER_INPUT

Entrées du joueur envoyées au serveur à chaque frame.

**Direction** : Client → Serveur

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | sequenceNum | uint32 | 4      | Numéro de séquence (détection perte/dupliqués) |
| 0x04   | tick        | uint32 | 4      | Tick client |
| 0x08   | moveX       | int8   | 1      | Axe X : -1 (gauche), 0, 1 (droite) |
| 0x09   | moveY       | int8   | 1      | Axe Y : -1 (haut), 0, 1 (bas) |
| 0x0A   | actions     | uint8  | 1      | Bitfield des actions |

**Actions (bitfield)** :
- Bit 0 (0x01) : Tir primaire
- Bit 1 (0x02) : Tir chargé
- Bit 2 (0x04) : Libérer la Force
- Bit 3 (0x08) : Rappeler la Force
- Bits 4-7 : Réservé

**Exemple** : Mouvement droite + tir
```
Header: 10 02 0B 00 00 00
Payload: 00 00 00 2A 00 00 01 F4 01 00 01
```

---

### 0x11 - GAME_STATE

État complet du jeu envoyé périodiquement par le serveur.

**Direction** : Serveur → Clients

| Offset | Champ           | Type   | Taille | Description |
|--------|-----------------|--------|--------|-------------|
| 0x00   | sequenceNum     | uint32 | 4      | Numéro de séquence (détection perte/dupliqués) |
| 0x04   | tick            | uint32 | 4      | Tick serveur actuel |
| 0x08   | playerCount     | uint8  | 1      | Nombre de joueurs |
| 0x09   | players[]       | struct | N×15   | États des joueurs |
| var    | enemyCount      | uint8  | 1      | Nombre d'ennemis |
| var    | enemies[]       | struct | N×16   | États des ennemis |
| var    | projectileCount | uint8  | 1      | Nombre de projectiles |
| var    | projectiles[]   | struct | N×18   | États des projectiles |

**Structure Player State** (15 bytes) :

| Offset | Champ    | Type   | Taille | Description |
|--------|----------|--------|--------|-------------|
| 0x00   | playerId | uint16 | 2      | ID du joueur |
| 0x02   | posX     | float  | 4      | Position X (IEEE 754) |
| 0x06   | posY     | float  | 4      | Position Y |
| 0x0A   | hp       | uint8  | 1      | Points de vie |
| 0x0B   | shield   | uint8  | 1      | Bouclier |
| 0x0C   | weapon   | uint8  | 1      | Arme équipée |
| 0x0D   | state    | uint8  | 1      | État (0=alive, 1=dead, 2=invincible) |
| 0x0E   | sprite   | uint8  | 1      | Index du sprite (couleur/apparence) |

**Structure Enemy State** (16 bytes) :

| Offset | Champ      | Type   | Taille | Description |
|--------|------------|--------|--------|-------------|
| 0x00   | enemyId    | uint16 | 2      | ID de l'ennemi |
| 0x02   | enemyType  | uint8  | 1      | Type d'ennemi (serpent, tourelle, etc.) |
| 0x03   | posX       | float  | 4      | Position X |
| 0x07   | posY       | float  | 4      | Position Y |
| 0x0B   | hp         | uint8  | 1      | Points de vie |
| 0x0C   | state      | uint8  | 1      | État (0=idle, 1=attacking, 2=dying) |
| 0x0D   | pattern    | uint8  | 1      | Pattern de mouvement actif |
| 0x0E   | direction  | int8   | 1      | Direction (-1, 0, 1) |
| 0x0F   | flags      | uint8  | 1      | Flags (hasBonus, etc.) |

**Structure Projectile State** (18 bytes) :

| Offset | Champ        | Type   | Taille | Description |
|--------|--------------|--------|--------|-------------|
| 0x00   | projectileId | uint16 | 2      | ID du projectile |
| 0x02   | ownerId      | uint16 | 2      | ID du propriétaire |
| 0x04   | type         | uint8  | 1      | Type de projectile |
| 0x05   | posX         | float  | 4      | Position X |
| 0x09   | posY         | float  | 4      | Position Y |
| 0x0D   | velX         | float  | 4      | Vélocité X |
| 0x11   | velY         | float  | 4      | Vélocité Y |
| 0x15   | damage       | uint8  | 1      | Dégâts |

---

### 0x12 - ENTITY_SPAWN

Notification de spawn d'une nouvelle entité.

**Direction** : Serveur → Clients

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | sequenceNum | uint32 | 4      | Numéro de séquence (détection perte/dupliqués) |
| 0x04   | entityId    | uint16 | 2      | ID de l'entité |
| 0x06   | entityType  | uint8  | 1      | Type (0=enemy, 1=powerup, 2=obstacle) |
| 0x07   | subType     | uint8  | 1      | Sous-type spécifique |
| 0x08   | posX        | float  | 4      | Position X de spawn |
| 0x0C   | posY        | float  | 4      | Position Y de spawn |

---

### 0x13 - ENTITY_DESTROY

Notification de destruction d'une entité.

**Direction** : Serveur → Clients

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | sequenceNum | uint32 | 4      | Numéro de séquence (détection perte/dupliqués) |
| 0x04   | entityId    | uint16 | 2      | ID de l'entité détruite |
| 0x06   | destroyType | uint8  | 1      | Type de destruction (0=killed, 1=timeout) |
| 0x07   | killerId    | uint16 | 2      | ID du joueur responsable (0=auto) |

---

### 0x14 - PLAYER_HIT

Notification qu'un joueur a été touché.

**Direction** : Serveur → Clients

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | sequenceNum | uint32 | 4      | Numéro de séquence (détection perte/dupliqués) |
| 0x04   | playerId    | uint16 | 2      | ID du joueur touché |
| 0x06   | damage      | uint8  | 1      | Dégâts infligés |
| 0x07   | attackerId  | uint16 | 2      | ID de l'attaquant |
| 0x09   | newHp       | uint8  | 1      | HP restants |

---

### 0x15 - POWERUP_COLLECTED

Notification de collecte d'un bonus.

**Direction** : Serveur → Clients

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | sequenceNum | uint32 | 4      | Numéro de séquence (détection perte/dupliqués) |
| 0x04   | playerId    | uint16 | 2      | ID du collecteur |
| 0x06   | powerupId   | uint16 | 2      | ID du bonus |
| 0x08   | powerupType | uint8  | 1      | Type de bonus (weapon, shield, etc.) |

---

### 0x16 - FORCE_UPDATE

État de la Force (module spécial R-Type).

**Direction** : Serveur → Clients

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | sequenceNum | uint32 | 4      | Numéro de séquence (détection perte/dupliqués) |
| 0x04   | playerId    | uint16 | 2      | ID du propriétaire |
| 0x06   | state       | uint8  | 1      | État (0=attached_front, 1=attached_back, 2=detached) |
| 0x07   | posX        | float  | 4      | Position X (si détachée) |
| 0x0B   | posY        | float  | 4      | Position Y (si détachée) |
| 0x0F   | level       | uint8  | 1      | Niveau de puissance (0-3) |

---

### 0x17 - BOSS_SPAWN

Notification d'apparition d'un boss.

**Direction** : Serveur → Clients

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | sequenceNum | uint32 | 4      | Numéro de séquence (détection perte/dupliqués) |
| 0x04   | bossId      | uint16 | 2      | ID du boss |
| 0x06   | bossType    | uint8  | 1      | Type (0=Dobkeratops, etc.) |
| 0x07   | maxHp       | uint16 | 2      | HP maximum |
| 0x09   | phase       | uint8  | 1      | Phase actuelle |

---

### 0x18 - BOSS_UPDATE

Mise à jour de l'état du boss.

**Direction** : Serveur → Clients

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | sequenceNum | uint32 | 4      | Numéro de séquence (détection perte/dupliqués) |
| 0x04   | bossId      | uint16 | 2      | ID du boss |
| 0x06   | posX        | float  | 4      | Position X |
| 0x0A   | posY        | float  | 4      | Position Y |
| 0x0E   | hp          | uint16 | 2      | HP actuels |
| 0x10   | phase       | uint8  | 1      | Phase actuelle |
| 0x11   | action      | uint8  | 1      | Action en cours |

---

### 0x19 - SCROLLING_UPDATE

Mise à jour de la position du scrolling (starfield).

**Direction** : Serveur → Clients

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | sequenceNum | uint32 | 4      | Numéro de séquence (détection perte/dupliqués) |
| 0x04   | scrollSpeed | float  | 4      | Vitesse de défilement |
| 0x08   | offsetX     | float  | 4      | Décalage horizontal actuel |

---

### 0x1A - ACK

Accusé de réception pour messages critiques.

**Direction** : Bidirectionnelle

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | sequenceNum | uint32 | 4      | Numéro de séquence acquitté |
| 0x04   | messageType | uint8  | 1      | Type du message original |

**Mécanisme** :
1. Émetteur marque le message avec flag 0x08 (Requires ACK)
2. Émetteur stocke le message et démarre un timer
3. Récepteur renvoie un ACK avec le sequenceNum
4. Si pas d'ACK après timeout : retransmission (max 3 tentatives)

---

## Gestion de la Fiabilité UDP

### Numéros de Séquence

Tous les messages UDP incluent un `sequenceNum` permettant de :
- **Détecter les paquets perdus** : Si un numéro de séquence est manquant
- **Ignorer les paquets dupliqués** : Si un numéro de séquence a déjà été reçu
- **Réordonner les paquets** : Si nécessaire pour traiter les messages dans l'ordre

Le `sequenceNum` est un compteur qui s'incrémente à chaque message envoyé. Il permet au récepteur de savoir si des paquets ont été perdus en route.

---

## Résumé des Types de Messages

### TCP (0x01 - 0x0F)

| Type | Nom                  | Description |
|------|----------------------|-------------|
| 0x01 | CLIENT_CONNECT       | Demande de connexion initiale |
| 0x02 | SERVER_WELCOME       | Confirmation de connexion |
| 0x03 | LOBBY_LIST           | Liste des parties disponibles |
| 0x04 | LOBBY_JOIN           | Demande de rejoindre un lobby |
| 0x05 | LOBBY_JOINED         | Confirmation d'entrée dans lobby |
| 0x06 | CHAT_MESSAGE         | Message textuel |
| 0x07 | GAME_START           | Démarrage de partie |
| 0x08 | GAME_END             | Fin de partie avec résultats |
| 0x09 | PLAYER_DISCONNECT    | Déconnexion d'un joueur |
| 0x0A | ERROR                | Message d'erreur |

### UDP (0x10 - 0x1F)

| Type | Nom                  | Description |
|------|----------------------|-------------|
| 0x10 | PLAYER_INPUT         | Entrées du joueur |
| 0x11 | GAME_STATE           | État complet du jeu |
| 0x12 | ENTITY_SPAWN         | Apparition d'une entité |
| 0x13 | ENTITY_DESTROY       | Destruction d'une entité |
| 0x14 | PLAYER_HIT           | Joueur touché |
| 0x15 | POWERUP_COLLECTED    | Collecte de bonus |
| 0x16 | FORCE_UPDATE         | État de la Force |
| 0x17 | BOSS_SPAWN           | Apparition d'un boss |
| 0x18 | BOSS_UPDATE          | Mise à jour du boss |
| 0x19 | SCROLLING_UPDATE     | Mise à jour du scrolling |
| 0x1A | ACK                  | Accusé de réception |
