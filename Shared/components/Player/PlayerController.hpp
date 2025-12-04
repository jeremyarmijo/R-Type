// Copyright 2025 Dalia Guiz
#pragma once

struct PlayerControlled {
    int player_id;
    Vector2 input;
    float speed;
};

struct Health {
    int hp;
    bool isAlive;
    float invtimer;
};

struct Weapon {
    int weaponId;
    bool hasForce;
};

struct PlayerId {
    int playerId;
    int team;
    int score;
};
