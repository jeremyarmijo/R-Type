# Documentation du Protocole Réseau R-Type

## Sommaire

1. [Choix TCP / UDP](#choix-tcp--udp)  
2. [Structure Commune des Messages](#structure-commune-des-messages)  
   - [Header (6 bytes)](#header-6-bytes)  
3. [Messages TCP](#messages-tcp)  
   - [0x01 - LOGIN_REQUEST](#0x01---login_request)  
   - [0x02 - LOGIN_RESPONSE](#0x02---login_response)  
   - [0x0F - GAME_START](#0x0f---game_start)  
   - [0x10 - GAME_END](#0x10---game_end)  
   - [0x12 - ERROR](#0x12---error)  
4. [Messages UDP (Temps Réel)](#messages-udp-temps-réel)  
   - [0x20 - PLAYER_INPUT](#0x20---player_input)  
   - [0x21 - GAME_STATE](#0x21---game_state)  
   - [0x22 - AUTH](#0x22---auth)  
   - [0x23 - BOSS_SPAWN](#0x23---boss_spawn)  
   - [0x24 - BOSS_UPDATE](#0x24---boss_update)  
   - [0x25 - ENEMY_HIT](#0x25---enemy_hit)  
5. [Gestion de la Fiabilité UDP](#gestion-de-la-fiabilité-udp)  
6. [Résumé des Types de Messages](#résumé-des-types-de-messages)  

---

### Choix TCP / UDP

- **TCP** : Communications critiques nécessitant une livraison garantie (authentification, chat, lobby, données de chunks, chargement)  
- **UDP** : Communications temps réel du gameplay (mouvements, tirs, états des entités, notifications de chunks)  

---

## Structure Commune des Messages

### Header (6 bytes)

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

---

## Messages TCP

### 0x01 - LOGIN_REQUEST

**Direction** : Client → Serveur  

| Offset | Champ        | Type   | Taille | Description |
|--------|--------------|--------|--------|-------------|
| 0x00   | usernameLen  | uint8  | 1      | Longueur du nom d'utilisateur |
| 0x01   | username     | char[] | N      | Nom d'utilisateur UTF-8 |
| 0x01+N | passwordLen  | uint8  | 1      | Longueur du mot de passe |
| 0x02+N | password     | char[] | N      | Mot de passe UTF-8 (hashé côté client) |

---

### 0x02 - LOGIN_RESPONSE

**Direction** : Serveur → Client

| Offset | Champ        | Type   | Taille | Description |
|--------|--------------|--------|--------|-------------|
| 0x00   | success      | uint8  | 1      | 1=succès, 0=échec |
| 0x01   | playerId     | uint16 | 2      | Identifiant unique attribué (si succès) |
| 0x03   | udpPort      | uint16 | 2      | Port UDP pour le gameplay |

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

### 0x0F - GAME_START

**Direction** : Serveur → Clients  

| Offset | Champ            | Type   | Taille | Description |
|--------|------------------|--------|--------|-------------|
| 0x00   | playerSpawnX     | float  | 4      | Position X de spawn du joueur |
| 0x04   | playerSpawnY     | float  | 4      | Position Y de spawn du joueur |
| 0x08   | scrollSpeed      | float  | 4      | Vitesse initiale du scrolling |

---

### 0x10 - GAME_END

**Direction** : Serveur → Clients  

| Offset | Champ        | Type   | Taille | Description |
|--------|--------------|--------|--------|-------------|
| 0x00   | victory      | uint8  | 1      | 1=victoire, 0=défaite |
| 0x01   | playerCount  | uint8  | 1      | Nombre de joueurs |
| 0x02   | scores[]     | struct | N×8    | Scores finaux |

#### Structure `scores[]` (8 bytes par joueur)

| Offset | Champ       | Type   | Taille | Description |
|--------|------------|--------|--------|-------------|
| 0x00   | playerId    | uint16 | 2      | Identifiant du joueur |
| 0x02   | score       | uint32 | 4      | Score final |
| 0x06   | rank        | uint8  | 1      | Classement |

---

### 0x12 - ERROR

**Direction** : Serveur → Client  

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | errorCode   | uint16 | 2      | Code d'erreur |
| 0x02   | messageLen  | uint8  | 1      | Longueur du message |
| 0x03   | message     | char[] | N      | Description de l'erreur |

---

## Messages UDP (Temps Réel)

### 0x20 - PLAYER_INPUT

**Direction** : Client → Serveur

| Offset | Champ       | Type   | Taille | Description                     |
|--------|-------------|--------|--------|---------------------------------|
| 0x00   | up          | uint8  | 1      | 1 si appuyé, 0 sinon           |
| 0x01   | down        | uint8  | 1      | 1 si appuyé, 0 sinon           |
| 0x02   | left        | uint8  | 1      | 1 si appuyé, 0 sinon           |
| 0x03   | right       | uint8  | 1      | 1 si appuyé, 0 sinon           |
| 0x04   | fire        | uint8  | 1      | Bitfield      |

**Fire (bitfield)** :
- Bit 0 (0x01) : Tir primaire
- Bit 1 (0x02) : Tir chargé

---

### 0x21 - GAME_STATE

**Direction** : Serveur → Clients  

| Offset | Champ           | Type   | Taille | Description |
|--------|-----------------|--------|--------|-------------|
| 0x00   | playerCount     | uint8  | 1      | Nombre de joueurs |
| 0x01   | players[]       | struct | N×15   | États des joueurs |
| ...    | enemyCount      | uint8  | 1      | Nombre d'ennemis |
| ...    | enemies[]       | struct | M×10   | États des ennemis |
| ...    | projectileCount | uint8  | 1      | Nombre de projectiles |
| ...    | projectiles[]   | struct | P×8    | États des projectiles |

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

### 0x22 - AUTH

**Direction** : Client → Serveur  

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | playerId    | uint16 | 2      | Identifiant unique du joueur |

---

### 0x23 - BOSS_SPAWN

**Direction** : Serveur → Clients  

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | bossId      | uint16 | 2      | ID du boss |
| 0x02   | bossType    | uint8  | 1      | Type |
| 0x03   | maxHp       | uint16 | 2      | HP maximum |
| 0x05   | phase       | uint8  | 1      | Phase actuelle |

---

### 0x24 - BOSS_UPDATE

**Direction** : Serveur → Clients  

| Offset | Champ       | Type   | Taille | Description |
|--------|-------------|--------|--------|-------------|
| 0x00   | bossId      | uint16 | 2      | ID du boss |
| 0x02   | posX        | float  | 4      | Position X |
| 0x06   | posY        | float  | 4      | Position Y |
| 0x0A   | hp          | uint16 | 2      | HP actuels |
| 0x0C   | phase       | uint8  | 1      | Phase actuelle |
| 0x0D   | action      | uint8  | 1      | Action en cours |

---

### 0x25 - ENEMY_HIT

**Direction** : Serveur → Clients  

| Offset | Champ        | Type   | Taille | Description |
|--------|--------------|--------|--------|-------------|
| 0x00   | enemyId      | uint16 | 2      | ID de l’ennemi touché |
| 0x02   | damage       | uint8  | 1      | Dégâts infligés |
| 0x03   | hpRemaining  | uint16 | 2      | Points de vie restants de l’ennemi |

---

## Gestion de la Fiabilité UDP

- Tous les messages UDP incluent un `sequenceNum` pour :  
  - Détecter paquets perdus  
  - Ignorer paquets dupliqués  
  - Réordonner les paquets  

---

## Résumé des Types de Messages

### TCP (0x01 - 0x12)

| Type | Nom            | Description |
|------|----------------|-------------|
| 0x01 | LOGIN_REQUEST  | Demande de connexion |
| 0x02 | LOGIN_RESPONSE | Réponse de connexion |
| 0x0F | GAME_START     | Démarrage de partie |
| 0x10 | GAME_END       | Fin de partie |
| 0x12 | ERROR          | Message d'erreur |

### UDP (0x20 - 0x25)

| Type | Nom            | Description |
|------|----------------|-------------|
| 0x20 | PLAYER_INPUT   | Entrées du joueur |
| 0x21 | GAME_STATE     | État complet du jeu (joueurs, ennemis, projectiles) |
| 0x22 | AUTH           | Authentification client → serveur |
| 0x23 | BOSS_SPAWN     | Apparition d'un boss |
| 0x24 | BOSS_UPDATE    | Mise à jour du boss |
| 0x25 | ENEMY_HIT      | Notification qu’un ennemi a été touché |
