# Documentation du Protocole Réseau R-Type

## Sommaire

1. [Choix TCP / UDP](#choix-tcp--udp)
2. [Structure Commune des Messages](#structure-commune-des-messages)
   - [Header (6 bytes)](#header-6-bytes)
3. [Messages TCP](#messages-tcp)
   - [0x01 - LOGIN_REQUEST](#0x01---login_request)
   - [0x02 - LOGIN_RESPONSE](#0x02---login_response)
   - [0x03 - SIGNUP_REQUEST](#0x03---signup_request)
   - [0x04 - SIGNUP_RESPONSE](#0x04---signup_response)
   - [0x05 - LOGOUT](#0x05---logout)
   - [0x06 - LOBBY_LIST](#0x06---lobby_list)
   - [0x07 - LOBBY_JOIN](#0x07---lobby_join)
   - [0x08 - LOBBY_JOINED](#0x08---lobby_joined)
   - [0x09 - CHAT_MESSAGE](#0x09---chat_message)
   - [0x0A - GAME_START](#0x0a---game_start)
   - [0x0B - GAME_END](#0x0b---game_end)
   - [0x0C - PLAYER_DISCONNECT](#0x0c---player_disconnect)
   - [0x0D - ERROR](#0x0d---error)
   - [0x0E - CHUNK_REQUEST](#0x0e---chunk_request)
   - [0x0F - CHUNK_DATA](#0x0f---chunk_data)
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
   - [0x1B - CHUNK_UNLOAD](#0x1b---chunk_unload)
   - [0x1C - CHUNK_TILE_UPDATE](#0x1c---chunk_tile_update)
   - [0x1D - CHUNK_VISIBILITY](#0x1d---chunk_visibility)
5. [Gestion de la Fiabilité UDP](#gestion-de-la-fiabilité-udp)
6. [Résumé des Types de Messages](#résumé-des-types-de-messages)

---

### Choix TCP / UDP

- **TCP** : Utilisé pour les communications critiques nécessitant une livraison garantie (authentification, connexion, chat, événements de lobby, données de chunks)
- **UDP** : Utilisé pour toutes les communications temps réel du gameplay (mouvements, tirs, états des entités, notifications de chunks)

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

### 0x01 - LOGIN_REQUEST

Demande de connexion avec identifiants existants.

**Direction** : Client → Serveur

| Offset | Champ        | Type   | Taille | Description |
|--------|--------------|--------|--------|-------------|
| 0x00   | usernameLen  | uint8  | 1      | Longueur du nom d'utilisateur (max 32) |
| 0x01   | username     | char[] | N      | Nom d'utilisateur en UTF-8 |
| var    | passwordLen  | uint8  | 1      | Longueur du mot de passe (max 64) |
| var    | password     | char[] | N      | Mot de passe en UTF-8 (hashé côté client) |

**Exemple** : Login de l'utilisateur "Falcon" avec mot de passe "hash123"
```
Header: 01 01 13 00 00 00
Payload: 06 46 61 6C 63 6F 6E 07 68 61 73 68 31 32 33
```

**Note de sécurité** : Le mot de passe doit être hashé côté client avant l'envoi (par exemple avec SHA-256).

---

### 0x02 - LOGIN_RESPONSE

Réponse du serveur à une demande de connexion.

**Direction** : Serveur → Client

| Offset | Champ        | Type   | Taille | Description |
|--------|--------------|--------|--------|-------------|
| 0x00   | success      | uint8  | 1      | 1=succès, 0=échec |
| 0x01   | playerId     | uint16 | 2      | Identifiant unique attribué (si succès) |
| 0x03   | serverTick   | uint32 | 4      | Tick actuel du serveur |
| 0x07   | udpPort      | uint16 | 2      | Port UDP pour le gameplay |

**Si échec (success = 0)** :

| Offset | Champ      | Type   | Taille | Description |
|--------|------------|--------|--------|-------------|
| 0x00   | success    | uint8  | 1      | 0=échec |
| 0x01   | errorCode  | uint16 | 2      | Code d'erreur |
| 0x03   | messageLen | uint8  | 1      | Longueur du message d'erreur |
| 0x04   | message    | char[] | N      | Message d'erreur |

**Codes d'erreur login** :
- `0x1001` : Nom d'utilisateur ou mot de passe incorrect
- `0x1002` : Compte banni
- `0x1003` : Déjà connecté ailleurs
- `0x1004` : Serveur plein

---

### 0x03 - SIGNUP_REQUEST

Demande de création de nouveau compte.

**Direction** : Client → Serveur

| Offset | Champ        | Type   | Taille | Description |
|--------|--------------|--------|--------|-------------|
| 0x00   | usernameLen  | uint8  | 1      | Longueur du nom d'utilisateur (max 32) |
| 0x01   | username     | char[] | N      | Nom d'utilisateur en UTF-8 |
| var    | passwordLen  | uint8  | 1      | Longueur du mot de passe (max 64) |
| var    | password     | char[] | N      | Mot de passe en UTF-8 (hashé côté client) |
| var    | emailLen     | uint8  | 1      | Longueur de l'email (max 128) |
| var    | email        | char[] | N      | Email en UTF-8 |

**Exemple** : Création du compte "Falcon"
```
Header: 03 01 2E 00 00 00
Payload: 06 46 61 6C 63 6F 6E 07 68 61 73 68 31 32 33 11 66 61 6C 63 6F 6E 40 65 6D 61 69 6C 2E 63 6F 6D
```

---

### 0x04 - SIGNUP_RESPONSE

Réponse du serveur à une demande de création de compte.

**Direction** : Serveur → Client

| Offset | Champ      | Type   | Taille | Description |
|--------|------------|--------|--------|-------------|
| 0x00   | success    | uint8  | 1      | 1=succès, 0=échec |
| 0x01   | messageLen | uint8  | 1      | Longueur du message |
| 0x02   | message    | char[] | N      | Message de confirmation ou d'erreur |

**Si échec (success = 0)** :

| Offset | Champ      | Type   | Taille | Description |
|--------|------------|--------|--------|-------------|
| 0x00   | success    | uint8  | 1      | 0=échec |
| 0x01   | errorCode  | uint16 | 2      | Code d'erreur |
| 0x03   | messageLen | uint8  | 1      | Longueur du message d'erreur |
| 0x04   | message    | char[] | N      | Message d'erreur |

**Codes d'erreur signup** :
- `0x2001` : Nom d'utilisateur déjà pris
- `0x2002` : Email déjà utilisé
- `0x2003` : Nom d'utilisateur invalide (caractères interdits)
- `0x2004` : Mot de passe trop faible
- `0x2005` : Email invalide

---

### 0x05 - LOGOUT

Demande de déconnexion propre.

**Direction** : Client → Serveur

| Offset | Champ    | Type   | Taille | Description |
|--------|----------|--------|--------|-------------|
| 0x00   | playerId | uint16 | 2      | ID du joueur qui se déconnecte |

**Exemple**
```
Header: 05 01 02 00 00 00
Payload: 00 01
```

---

### 0x06 - LOBBY_LIST

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

### 0x07 - LOBBY_JOIN

Demande de rejoindre un lobby.

**Direction** : Client → Serveur

| Offset | Champ   | Type   | Taille | Description |
|--------|---------|--------|--------|-------------|
| 0x00   | lobbyId | uint16 | 2      | ID du lobby à rejoindre |

---

### 0x08 - LOBBY_JOINED

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

### 0x09 - CHAT_MESSAGE

Message textuel dans le lobby ou en jeu.

**Direction** : Bidirectionnelle

| Offset | Champ      | Type   | Taille | Description |
|--------|------------|--------|--------|-------------|
| 0x00   | senderId   | uint16 | 2      | ID de l'émetteur (0 = serveur) |
| 0x02   | messageLen | uint16 | 2      | Longueur du message (max 256) |
| 0x04   | message    | char[] | N      | Contenu UTF-8 |

---

### 0x0A - GAME_START

Notification du démarrage d'une partie.

**Direction** : Serveur → Clients

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | mapId       | uint16 | 2      | ID de la map/niveau |
| 0x02   | gameMode    | uint8  | 1      | Mode de jeu (0=coop, 1=versus) |
| 0x03   | difficulty  | uint8  | 1      | Difficulté (0-3) |
| 0x04   | startTick   | uint32 | 4      | Tick de démarrage |
| 0x08   | mapWidth    | uint32 | 4      | Largeur totale de la map (en pixels) |
| 0x0C   | mapHeight   | uint16 | 2      | Hauteur de la map (en pixels) |
| 0x0E   | chunkSize   | uint16 | 2      | Taille d'un chunk (en pixels) |

---

### 0x0B - GAME_END

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

### 0x0C - PLAYER_DISCONNECT

Notification de déconnexion d'un joueur.

**Direction** : Serveur → Clients

| Offset | Champ    | Type   | Taille | Description |
|--------|----------|--------|--------|-------------|
| 0x00   | playerId | uint16 | 2      | ID du joueur déconnecté |
| 0x02   | reason   | uint8  | 1      | Raison (0=normal, 1=timeout, 2=kick) |

---

### 0x0D - ERROR

Message d'erreur du serveur.

**Direction** : Serveur → Client

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | errorCode   | uint16 | 2      | Code d'erreur |
| 0x02   | messageLen  | uint8  | 1      | Longueur du message |
| 0x03   | message     | char[] | N      | Description de l'erreur |

**Codes d'erreur généraux** :
- `0x0001` : Lobby plein
- `0x0002` : Pseudo déjà utilisé
- `0x0003` : Version du protocole incompatible
- `0x0004` : Serveur plein
- `0x0005` : Lobby inexistant
- `0x0006` : Non autorisé
- `0x0007` : Action non permise
- `0x0008` : Chunk inexistant
- `0x0009` : Chunk déjà chargé

---

### 0x0E - CHUNK_REQUEST

Demande d'un chunk de map spécifique.

**Direction** : Client → Serveur

| Offset | Champ   | Type  | Taille | Description |
|--------|---------|-------|--------|-------------|
| 0x00   | chunkX  | int32 | 4      | Coordonnée X du chunk demandé |

**Exemple** : Demande du chunk X=5
```
Header: 0E 01 04 00 00 00
Payload: 00 00 00 05
```

---

### 0x0F - CHUNK_DATA

Données d'un chunk de map.

**Direction** : Serveur → Client

| Offset | Champ        | Type   | Taille | Description |
|--------|--------------|--------|--------|-------------|
| 0x00   | chunkX       | int32  | 4      | Coordonnée X du chunk |
| 0x04   | chunkWidth   | uint16 | 2      | Largeur du chunk (en tiles) |
| 0x06   | chunkHeight  | uint16 | 2      | Hauteur du chunk (en tiles) |
| 0x08   | tileCount    | uint32 | 4      | Nombre de tiles dans le chunk |
| 0x0C   | tiles[]      | struct | N×8    | Données des tiles |

**Structure Tile** (8 bytes) :

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | tileX       | uint16 | 2      | Position X locale dans le chunk |
| 0x02   | tileY       | uint16 | 2      | Position Y dans le chunk (hauteur de l'écran) |
| 0x04   | tileType    | uint8  | 1      | Type de tile (0=vide, 1=obstacle, 2=destructible, etc.) |
| 0x05   | tileSprite  | uint8  | 1      | Index du sprite à afficher |
| 0x06   | tileFlags   | uint8  | 1      | Flags (collidable, destructible, etc.) |
| 0x07   | tileHealth  | uint8  | 1      | Points de vie (si destructible) |

**Flags des tiles** :
- Bit 0 (0x01) : Collidable (bloque les entités)
- Bit 1 (0x02) : Destructible (peut être détruit)
- Bit 2 (0x04) : Animated (tile animée)
- Bit 3 (0x08) : Background (arrière-plan)
- Bits 4-7 : Réservé

**Types de tiles** :
- `0x00` : Vide (pas de collision)
- `0x01` : Obstacle solide
- `0x02` : Tile destructible
- `0x03` : Plateforme
- `0x04` : Danger (lave, piques, etc.)
- `0x05` : Checkpoint

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

### 0x1B - CHUNK_UNLOAD

Notification qu'un chunk peut être déchargé de la mémoire.

**Direction** : Serveur → Clients

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | sequenceNum | uint32 | 4      | Numéro de séquence (détection perte/dupliqués) |
| 0x04   | chunkX      | int32  | 4      | Coordonnée X du chunk à décharger |

**Exemple** : Déchargement du chunk X=2
```
Header: 1B 02 08 00 00 00
Payload: 00 00 00 15 00 00 00 02
```

---

### 0x1C - CHUNK_TILE_UPDATE

Mise à jour d'une tile spécifique dans un chunk (ex: tile détruite).

**Direction** : Serveur → Clients

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | sequenceNum | uint32 | 4      | Numéro de séquence (détection perte/dupliqués) |
| 0x04   | chunkX      | int32  | 4      | Coordonnée X du chunk |
| 0x08   | tileX       | uint16 | 2      | Position X locale de la tile |
| 0x0A   | tileY       | uint16 | 2      | Position Y de la tile |
| 0x0C   | newTileType | uint8  | 1      | Nouveau type de tile |
| 0x0D   | newHealth   | uint8  | 1      | Nouveaux points de vie |

**Exemple** : Tile détruite au chunk X=5, tile locale (10, 15)
```
Header: 1C 02 0E 00 00 00
Payload: 00 00 00 20 00 00 00 05 00 0A 00 0F 00 00
```

---

### 0x1D - CHUNK_VISIBILITY

Liste des chunks visibles pour un joueur (optimisation du streaming).

**Direction** : Serveur → Client

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | sequenceNum | uint32 | 4      | Numéro de séquence (détection perte/dupliqués) |
| 0x04   | chunkCount  | uint8  | 1      | Nombre de chunks visibles |
| 0x05   | chunks[]    | int32  | N×4    | Liste des coordonnées X des chunks visibles |

**Exemple** : Chunks 3, 4, 5, 6, 7 visibles
```
Header: 1D 02 19 00 00 00
Payload: 00 00 00 25 05 00 00 00 03 00 00 00 04 00 00 00 05 00 00 00 06 00 00 00 07
```

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