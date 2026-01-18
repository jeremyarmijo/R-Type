#include "scenes/SecondGame.hpp"
#include "ecs/Zipper.hpp"
#include "scene/SceneManager.hpp"
#include "ecs/Registry.hpp"
#include "network/DataMask.hpp"
#include "input/InputSubsystem.hpp"
#include "network/Action.hpp"
#include "network/Event.hpp"

SecondGame::SecondGame() { 
    m_name = "secondgame"; 
}

void SecondGame::OnEnter() {
    SceneData& data = GetSceneData();
    auto players_list = data.Get<std::vector<std::tuple<uint16_t, bool, std::string>>>(
        "players_list",
        std::vector<std::tuple<uint16_t, bool, std::string>>()
    );
    
    // Register components
    GetRegistry().register_component<Transform>();
    GetRegistry().register_component<RigidBody>();
    GetRegistry().register_component<BoxCollider>();
    GetRegistry().register_component<PlayerEntity>();
    GetRegistry().register_component<InputState>();
    GetRegistry().register_component<FighterState>();
    GetRegistry().register_component<FighterStats>();
    GetRegistry().register_component<HitBox>();
    
    // Create fighters (max 2 players for 1v1)
    int playerCount = 0;
    for (auto& [playerId, ready, _] : players_list) {
        if (playerCount >= 2) break;
        
        // Spawn positions (left and right side of arena)
        float spawnX = (playerCount == 0) ? 200.0f : 600.0f;
        float spawnY = ARENA_FLOOR;
        
        Entity fighter = GetRegistry().spawn_entity();
        
        // Transform
        GetRegistry().emplace_component<Transform>(
            fighter, 
            Vector2{spawnX, spawnY}, 
            Vector2{2.0f, 2.0f}, 
            0.0f
        );
        
        // RigidBody
        GetRegistry().emplace_component<RigidBody>(
            fighter,
            1.0f,  // mass
            0.0f,  // restitution (no bounce)
            false  // not static
        );
        
        // BoxCollider (fighter body)
        GetRegistry().emplace_component<BoxCollider>(
            fighter,
            30.0f,  // width
            60.0f,  // height
            Vector2{0, 0}
        );
        
        // PlayerEntity
        GetRegistry().emplace_component<PlayerEntity>(
            fighter,
            250.0f  // move speed
        );
        auto& playerComp = GetRegistry().get_components<PlayerEntity>()[fighter];
        playerComp->player_id = playerId;
        playerComp->current = 100;  // health
        playerComp->max = 100;
        playerComp->isAlive = true;
        
        // InputState
        GetRegistry().emplace_component<InputState>(fighter);
        
        // FighterState
        GetRegistry().emplace_component<FighterState>(fighter);
        auto& fighterState = GetRegistry().get_components<FighterState>()[fighter];
        fighterState->facingRight = (playerCount == 0);
        
        // FighterStats
        GetRegistry().emplace_component<FighterStats>(fighter);
        
        // HitBox (initially inactive)
        GetRegistry().emplace_component<HitBox>(fighter);
        
        m_players[playerId] = fighter;
        roundWins[playerId] = 0;
        playerCount++;
    }
    
    std::cout << "[StreetFighter] Game initialized with " << m_players.size() << " fighters" << std::endl;
    
    // Start first round
    StartNewRound();
}

void SecondGame::OnExit() {
    m_players.clear();
    roundWins.clear();
}

void SecondGame::Update(float deltaTime) {
    ReceivePlayerInputs();
    
    if (roundStartDelay > 0.0f) {
        roundStartDelay -= deltaTime;
        if (roundStartDelay <= 0.0f) {
            roundActive = true;
            std::cout << "[Round " << roundNumber << "] FIGHT!" << std::endl;
        }
        BuildCurrentState();
        return;
    }
    
    if (roundActive) {
        roundTimer -= deltaTime;
        if (roundTimer <= 0.0f) {
            roundTimer = 0.0f;
            CheckRoundEnd();
        }
        
        UpdateFighterStates(deltaTime);
        UpdatePhysics(deltaTime);
        CheckCombat();
        CheckRoundEnd();
    }
    
    BuildCurrentState();
}

void SecondGame::ReceivePlayerInputs() {
    SceneData& data = GetSceneData();
    
    if (data.Has("last_input_event")) {
        Event event = data.Get<Event>("last_input_event");
        uint16_t playerId = data.Get<uint16_t>("last_input_player");
        
        if (event.type == EventType::PLAYER_INPUT) {
            const PLAYER_INPUT& input = std::get<PLAYER_INPUT>(event.data);
            
            auto& players = GetRegistry().get_components<PlayerEntity>();
            auto& states = GetRegistry().get_components<InputState>();
            
            for (auto&& [player, state] : Zipper(players, states)) {
                if (player.player_id != playerId) continue;
                
                state.moveLeft = input.left;
                state.moveRight = input.right;
                state.moveDown = input.down;  // crouch
                state.moveUp = input.up;      // jump
                state.action1 = (input.fire == 1);   // light attack
                state.action2 = (input.fire == 2);  // medium attack
                state.action3 = (input.fire == 3);  // heavy attack
                state.block = (input.fire == 4);      // block
                break;
            }
        }
        
        data.Remove("last_input_event");
        data.Remove("last_input_player");
    }
}

void SecondGame::UpdateFighterStates(float deltaTime) {
    auto& transforms = GetRegistry().get_components<Transform>();
    auto& rigidbodies = GetRegistry().get_components<RigidBody>();
    auto& players = GetRegistry().get_components<PlayerEntity>();
    auto& states = GetRegistry().get_components<InputState>();
    auto& fighterStates = GetRegistry().get_components<FighterState>();
    auto& fighterStats = GetRegistry().get_components<FighterStats>();
    auto& hitboxes = GetRegistry().get_components<HitBox>();
    
    for (auto&& [transform, rb, player, state, fstate, stats, hitbox] : 
         Zipper(transforms, rigidbodies, players, states, fighterStates, fighterStats, hitboxes)) {
        
        if (!player.isAlive) continue;
        
        fstate.stateTimer += deltaTime;
        
        // Update facing direction based on opponent position
        for (auto&& [otherTransform, otherPlayer] : Zipper(transforms, players)) {
            if (otherPlayer.player_id == player.player_id) continue;
            fstate.facingRight = (otherTransform.position.x > transform.position.x);
        }
        
        // State machine
        switch (fstate.currentState) {
            case FighterState::IDLE:
                hitbox.active = false;
                
                if (!stats.isGrounded) {
                    fstate.currentState = FighterState::JUMPING;
                } else if (state.moveDown) {
                    fstate.currentState = FighterState::CROUCHING;
                } else if (state.block) {
                    fstate.currentState = FighterState::BLOCKING;
                } else if (state.action1) {
                    fstate.currentState = FighterState::ATTACKING_LIGHT;
                    fstate.stateTimer = 0.0f;
                } else if (state.action2) {
                    fstate.currentState = FighterState::ATTACKING_MEDIUM;
                    fstate.stateTimer = 0.0f;
                } else if (state.action3) {
                    fstate.currentState = FighterState::ATTACKING_HEAVY;
                    fstate.stateTimer = 0.0f;
                } else if (state.moveLeft || state.moveRight) {
                    fstate.currentState = FighterState::WALKING;
                } else if (state.moveUp && stats.isGrounded) {
                    rb.velocity.y = -stats.jumpForce;
                    stats.isGrounded = false;
                    fstate.currentState = FighterState::JUMPING;
                }
                break;
                
            case FighterState::WALKING:
                hitbox.active = false;
                
                if (state.moveLeft && transform.position.x > ARENA_LEFT) {
                    rb.velocity.x = -stats.moveSpeed;
                } else if (state.moveRight && transform.position.x < ARENA_RIGHT) {
                    rb.velocity.x = stats.moveSpeed;
                } else {
                    rb.velocity.x = 0;
                }
                
                if (!state.moveLeft && !state.moveRight) {
                    fstate.currentState = FighterState::IDLE;
                } else if (state.action1 || state.action2 || state.action3) {
                    fstate.currentState = FighterState::IDLE;
                }
                break;
                
            case FighterState::CROUCHING:
                hitbox.active = false;
                rb.velocity.x = 0;
                
                if (!state.moveDown) {
                    fstate.currentState = FighterState::IDLE;
                }
                break;
                
            case FighterState::JUMPING:
                hitbox.active = false;
                
                // Air control
                if (state.moveLeft && transform.position.x > ARENA_LEFT) {
                    rb.velocity.x = -stats.moveSpeed * 0.5f;
                } else if (state.moveRight && transform.position.x < ARENA_RIGHT) {
                    rb.velocity.x = stats.moveSpeed * 0.5f;
                }
                
                if (stats.isGrounded) {
                    fstate.currentState = FighterState::IDLE;
                }
                break;
                
            case FighterState::BLOCKING:
                hitbox.active = false;
                rb.velocity.x = 0;
                
                if (!state.block) {
                    fstate.currentState = FighterState::IDLE;
                }
                break;
                
            case FighterState::ATTACKING_LIGHT:
                rb.velocity.x = 0;
                
                if (fstate.stateTimer < 0.1f) {
                    // Startup
                    hitbox.active = false;
                } else if (fstate.stateTimer < 0.2f) {
                    // Active frames
                    hitbox.active = true;
                    hitbox.damage = 5;
                    hitbox.stunDuration = 0.2f;
                    hitbox.width = 40.0f;
                    hitbox.height = 30.0f;
                    hitbox.offset = fstate.facingRight ? Vector2{30, 0} : Vector2{-30, 0};
                } else if (fstate.stateTimer >= 0.3f) {
                    // Recovery
                    hitbox.active = false;
                    fstate.currentState = FighterState::IDLE;
                    fstate.stateTimer = 0.0f;
                }
                break;
                
            case FighterState::ATTACKING_MEDIUM:
                rb.velocity.x = 0;
                
                if (fstate.stateTimer < 0.15f) {
                    hitbox.active = false;
                } else if (fstate.stateTimer < 0.3f) {
                    hitbox.active = true;
                    hitbox.damage = 10;
                    hitbox.stunDuration = 0.4f;
                    hitbox.width = 50.0f;
                    hitbox.height = 40.0f;
                    hitbox.offset = fstate.facingRight ? Vector2{35, 0} : Vector2{-35, 0};
                } else if (fstate.stateTimer >= 0.5f) {
                    hitbox.active = false;
                    fstate.currentState = FighterState::IDLE;
                    fstate.stateTimer = 0.0f;
                }
                break;
                
            case FighterState::ATTACKING_HEAVY:
                rb.velocity.x = 0;
                
                if (fstate.stateTimer < 0.25f) {
                    hitbox.active = false;
                } else if (fstate.stateTimer < 0.4f) {
                    hitbox.active = true;
                    hitbox.damage = 20;
                    hitbox.stunDuration = 0.6f;
                    hitbox.width = 60.0f;
                    hitbox.height = 50.0f;
                    hitbox.offset = fstate.facingRight ? Vector2{40, 0} : Vector2{-40, 0};
                } else if (fstate.stateTimer >= 0.7f) {
                    hitbox.active = false;
                    fstate.currentState = FighterState::IDLE;
                    fstate.stateTimer = 0.0f;
                }
                break;
                
            case FighterState::HIT_STUN:
                hitbox.active = false;
                rb.velocity.x = 0;
                
                if (fstate.stateTimer >= 0.5f) {
                    fstate.currentState = FighterState::IDLE;
                    fstate.stateTimer = 0.0f;
                }
                break;
                
            case FighterState::KNOCKED_DOWN:
                hitbox.active = false;
                rb.velocity.x = 0;
                
                if (fstate.stateTimer >= 1.5f) {
                    fstate.currentState = FighterState::IDLE;
                    fstate.stateTimer = 0.0f;
                }
                break;
        }
    }
}

void SecondGame::UpdatePhysics(float deltaTime) {
    auto& transforms = GetRegistry().get_components<Transform>();
    auto& rigidbodies = GetRegistry().get_components<RigidBody>();
    auto& stats = GetRegistry().get_components<FighterStats>();
    
    const Vector2 gravity = {0, 980.0f};
    
    for (auto&& [transform, rb, stat] : Zipper(transforms, rigidbodies, stats)) {
        // Apply gravity
        rb.velocity.y += gravity.y * deltaTime;
        
        // Update position
        transform.position += rb.velocity * deltaTime;
        
        // Ground collision
        if (transform.position.y >= ARENA_FLOOR) {
            transform.position.y = ARENA_FLOOR;
            rb.velocity.y = 0;
            stat.isGrounded = true;
        } else {
            stat.isGrounded = false;
        }
        
        // Arena bounds
        if (transform.position.x < ARENA_LEFT) {
            transform.position.x = ARENA_LEFT;
            rb.velocity.x = 0;
        } else if (transform.position.x > ARENA_RIGHT) {
            transform.position.x = ARENA_RIGHT;
            rb.velocity.x = 0;
        }
    }
}

void SecondGame::CheckCombat() {
    auto& transforms = GetRegistry().get_components<Transform>();
    auto& players = GetRegistry().get_components<PlayerEntity>();
    auto& fighterStates = GetRegistry().get_components<FighterState>();
    auto& hitboxes = GetRegistry().get_components<HitBox>();
    
    std::vector<Entity> fighters;
    for (size_t i = 0; i < players.size(); ++i) {
        if (players[i].has_value()) {
            fighters.push_back(GetRegistry().entity_from_index(i));
        }
    }
    
    // Check all fighter pairs
    for (size_t i = 0; i < fighters.size(); ++i) {
        for (size_t j = i + 1; j < fighters.size(); ++j) {
            Entity attacker = fighters[i];
            Entity defender = fighters[j];
            
            auto& attackerHitbox = hitboxes[attacker].value();
            auto& attackerTransform = transforms[attacker].value();
            auto& attackerFState = fighterStates[attacker].value();
            
            auto& defenderTransform = transforms[defender].value();
            auto& defenderPlayer = players[defender].value();
            auto& defenderFState = fighterStates[defender].value();
            
            // Check if attacker's hitbox is active
            if (!attackerHitbox.active) continue;
            
            // Calculate hitbox position
            Vector2 hitboxPos = attackerTransform.position + attackerHitbox.offset;
            
            // Simple AABB collision with defender
            float dx = std::abs(hitboxPos.x - defenderTransform.position.x);
            float dy = std::abs(hitboxPos.y - defenderTransform.position.y);
            
            if (dx < (attackerHitbox.width / 2 + 15) && 
                dy < (attackerHitbox.height / 2 + 30)) {
                
                // Hit landed!
                if (defenderFState.currentState != FighterState::BLOCKING) {
                    // Full damage
                    defenderPlayer.current -= attackerHitbox.damage;
                    defenderFState.currentState = FighterState::HIT_STUN;
                    defenderFState.stateTimer = 0.0f;
                    
                    std::cout << "[Combat] Player " << defenderPlayer.player_id 
                              << " hit for " << attackerHitbox.damage 
                              << " damage! HP: " << defenderPlayer.current << std::endl;
                } else {
                    // Blocked - reduced damage
                    int blockDamage = attackerHitbox.damage / 4;
                    defenderPlayer.current -= blockDamage;
                    
                    std::cout << "[Combat] Player " << defenderPlayer.player_id 
                              << " blocked! Chip damage: " << blockDamage << std::endl;
                }
                
                // Deactivate hitbox after hit
                attackerHitbox.active = false;
                
                if (defenderPlayer.current <= 0) {
                    defenderPlayer.current = 0;
                    defenderPlayer.isAlive = false;
                    defenderFState.currentState = FighterState::KNOCKED_DOWN;
                    defenderFState.stateTimer = 0.0f;
                }
            }
        }
    }
}

void SecondGame::CheckRoundEnd() {
    SceneData& data = GetSceneData();
    auto& players = GetRegistry().get_components<PlayerEntity>();
    
    int alivePlayers = 0;
    uint16_t winnerId = 0;
    
    for (auto&& [player] : Zipper(players)) {
        if (player.isAlive) {
            alivePlayers++;
            winnerId = player.player_id;
        }
    }
    
    // Round ends if only one player alive or time runs out
    if (alivePlayers <= 1 || roundTimer <= 0.0f) {
        roundActive = false;
        
        if (alivePlayers == 1) {
            roundWins[winnerId]++;
            std::cout << "[Round " << roundNumber << "] Player " << winnerId << " wins!" << std::endl;
        } else {
            // Time out - judge by health
            uint16_t healthWinner = 0;
            int maxHealth = -1;
            
            for (auto&& [player] : Zipper(players)) {
                if (player.current > maxHealth) {
                    maxHealth = player.current;
                    healthWinner = player.player_id;
                }
            }
            
            if (maxHealth > 0) {
                roundWins[healthWinner]++;
                std::cout << "[Round " << roundNumber << "] Time out! Player " 
                          << healthWinner << " wins by health!" << std::endl;
            }
        }
        
        // Check if match is over
        for (auto& [playerId, wins] : roundWins) {
            if (wins >= 2) {
                std::cout << "[Match] Player " << playerId << " wins the match!" << std::endl;
                data.Set("game_ended", true);
                data.Set("victory", true);
                data.Set("winner_id", playerId);
                return;
            }
        }
        
        // Start next round
        roundNumber++;
        if (roundNumber <= MAX_ROUNDS) {
            StartNewRound();
        }
    }
}

void SecondGame::StartNewRound() {
    roundTimer = 99.0f;
    roundStartDelay = 3.0f;
    roundActive = false;
    
    auto& transforms = GetRegistry().get_components<Transform>();
    auto& rigidbodies = GetRegistry().get_components<RigidBody>();
    auto& players = GetRegistry().get_components<PlayerEntity>();
    auto& fighterStates = GetRegistry().get_components<FighterState>();
    
    int playerIndex = 0;
    for (auto&& [transform, rb, player, fstate] : 
         Zipper(transforms, rigidbodies, players, fighterStates)) {
        
        // Reset positions
        float spawnX = (playerIndex == 0) ? 200.0f : 600.0f;
        transform.position = Vector2{spawnX, ARENA_FLOOR};
        rb.velocity = {0, 0};
        
        // Reset health and state
        player.current = player.max;
        player.isAlive = true;
        fstate.currentState = FighterState::IDLE;
        fstate.stateTimer = 0.0f;
        
        playerIndex++;
    }
    
    std::cout << "[Round " << roundNumber << "] Ready..." << std::endl;
}

void SecondGame::BuildCurrentState() {
    auto& transforms = GetRegistry().get_components<Transform>();
    auto& players = GetRegistry().get_components<PlayerEntity>();
    auto& fighterStates = GetRegistry().get_components<FighterState>();
    
    GameState gs;
    
    for (auto&& [idx, player, transform, fstate] : 
         IndexedZipper(players, transforms, fighterStates)) {
        
        Entity e = GetRegistry().entity_from_index(idx);
        if (!GetRegistry().is_entity_valid(e)) continue;
        
        PlayerState ps;
        ps.playerId = player.player_id;
        ps.posX = transform.position.x;
        ps.posY = transform.position.y;
        ps.hp = static_cast<uint8_t>(player.current);
        ps.state = static_cast<uint8_t>(fstate.currentState);
        ps.sprite = fstate.facingRight ? 1 : 0;  // facing direction
        ps.score = roundWins[player.player_id];
        
        ps.mask = M_POS_X | M_POS_Y | M_HP | M_STATE | M_SPRITE | M_SCORE;
        
        gs.players.push_back(ps);
    }
    
    SceneData& data = GetSceneData();
    data.Set("game_state", gs);
    data.Set("round_number", roundNumber);
    data.Set("round_timer", static_cast<int>(roundTimer));
}

void SecondGame::HandleEvent(SDL_Event& event) {}

extern "C" {
    Scene* CreateScene() {
        return new SecondGame();
    }
}
