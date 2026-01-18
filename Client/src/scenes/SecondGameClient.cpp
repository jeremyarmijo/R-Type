#include "scenes/SecondGameClient.hpp"
#include "ecs/Zipper.hpp"
#include "scene/SceneManager.hpp"
#include "ecs/Registry.hpp"
#include "network/Event.hpp"

SecondGameClient::SecondGameClient() { 
    m_name = "secondgame_client"; 
}

void SecondGameClient::OnEnter() {
    std::cout << "\n=== ENTERING STREET FIGHTER CLIENT SCENE ===" << std::endl;
    
    try {
        m_fighters.clear();
        m_fighterStates.clear();
        m_fighterInterpolation.clear();
        m_roundWins.clear();
        m_isInitialized = false;
        
        GetRegistry().register_component<Transform>();
        GetRegistry().register_component<Sprite>();
        
        m_localPlayerId = GetSceneData().Get<uint16_t>("playerId", 0);
        m_isSpectator = GetSceneData().Get<bool>("isSpectator", false);
        
        std::cout << "Player ID: " << m_localPlayerId << ", Spectator: " << m_isSpectator << std::endl;
        
        LoadFighterTextures();
        CreateFighterAnimations();
        
        CreateArenaVisuals();
        
        GetAudio()->PlayMusic("fight_music");
        
        m_roundText = GetUI()->AddElement<UIText>(140, 50, "ROUND 1", "", 40);
        m_roundText->SetLayer(100);
        
        m_timerText = GetUI()->AddElement<UIText>(140, 100, "99", "", 35);
        m_timerText->SetLayer(100);
        
        m_player1HealthText = GetUI()->AddElement<UIText>(50, 20, "P1 HP: 100", "", 25);
        m_player1HealthText->SetLayer(100);
        
        m_player2HealthText = GetUI()->AddElement<UIText>(480, 20, "P2 HP: 100", "", 25);
        m_player2HealthText->SetLayer(100);
        
        m_player1WinsText = GetUI()->AddElement<UIText>(50, 50, "Wins: 0", "", 20);
        m_player1WinsText->SetLayer(100);
        
        m_player2WinsText = GetUI()->AddElement<UIText>(480, 50, "Wins: 0", "", 20);
        m_player2WinsText->SetLayer(100);
        
        m_fightText = GetUI()->AddElement<UIText>(140, 300, "FIGHT!", "", 60);
        m_fightText->SetLayer(101);
        m_fightText->SetVisible(false);
        
        m_spectatorText = GetUI()->AddElement<UIText>(140, 200, "SPECTATOR MODE", "", 50);
        m_spectatorText->SetLayer(102);
        m_spectatorText->SetVisible(m_isSpectator);
        
        m_isInitialized = true;
        std::cout << "Street Fighter client initialized successfully" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "CRITICAL ERROR in OnEnter: " << e.what() << std::endl;
        m_isInitialized = false;
    }
    
    std::cout << "=================================\n" << std::endl;
}

void SecondGameClient::OnExit() {
    std::cout << "\n=== EXITING STREET FIGHTER CLIENT SCENE ===" << std::endl;
    
    m_fighters.clear();
    m_fighterStates.clear();
    m_fighterInterpolation.clear();
    m_roundWins.clear();
    m_healthBars.clear();
    m_isInitialized = false;
    
    GetUI()->Clear();
    
    std::cout << "Street Fighter client cleanup complete" << std::endl;
    std::cout << "==============================\n" << std::endl;
}

void SecondGameClient::Update(float deltaTime) {
    if (!m_isInitialized) return;
    
    if (!m_isSpectator) {
        player_input_system(GetRegistry(), GetInput(), GetNetwork());
    }
    
    InterpolateFighters(deltaTime);
    
    GetGameEvents(deltaTime);

    UpdateUI();

    if (m_fightTextTimer > 0.0f) {
        m_fightTextTimer -= deltaTime;
        if (m_fightTextTimer <= 0.0f) {
            m_fightText->SetVisible(false);
        }
    }
}

void SecondGameClient::Render() {
    if (!m_isInitialized) return;
}

void SecondGameClient::HandleEvent(SDL_Event& event) {
}

void SecondGameClient::LoadFighterTextures() {
    if (!GetRendering()->GetTexture("fighter1")) {
        GetRendering()->LoadTexture("fighter1", "../assets/enemy4.png");
    }
}

void SecondGameClient::CreateFighterAnimations() {
    GetRendering()->CreateAnimation("fighter1_idle", "fighter1",
        {{{282, 14, 47, 41}, 0.15f}}, true);
}

void SecondGameClient::CreateArenaVisuals() {
    m_arenaBackground = CreateSprite(GetRegistry(), "arena_bg", {640, 360}, -20);
}

void SecondGameClient::SpawnFighter(uint16_t playerId, Vector2 position, bool facingRight) {
    if (m_fighters.find(playerId) != m_fighters.end()) {
        std::cout << "Fighter " << playerId << " already spawned" << std::endl;
        return;
    }

    std::string fighterAnim = "fighter1_idle";
    std::string fighterTex = "fighter1";
    
    std::cout << "Spawning fighter " << playerId << " at (" << position.x 
              << ", " << position.y << ") facing " 
              << (facingRight ? "right" : "left") << std::endl;
    
    Entity fighter = CreateAnimatedSprite(GetRegistry(), 
                                         GetRendering()->GetAnimation(fighterAnim),
                                         fighterTex, position, fighterAnim, 0);
    
    auto& transform = GetRegistry().get_components<Transform>()[fighter];
    if (transform) {
        transform->scale = {3.0f, 3.0f};
        if (!facingRight) {
            transform->scale.x = -3.0f;
        }
    }
    
    m_fighters[playerId] = fighter;
    m_roundWins[playerId] = 0;
    
    m_fighterInterpolation[playerId] = {position, position, 0.0f};
    
    std::cout << "Fighter " << playerId << " spawned successfully" << std::endl;
}

void SecondGameClient::RemoveFighter(uint16_t playerId) {
    auto it = m_fighters.find(playerId);
    if (it != m_fighters.end()) {
        Entity fighter = it->second;
        GetRegistry().kill_entity(fighter);
        m_fighters.erase(it);
        m_fighterInterpolation.erase(playerId);
        
        std::cout << "Removed fighter " << playerId << std::endl;
    }
}

void SecondGameClient::UpdateFighterPosition(uint16_t playerId, Vector2 targetPosition) {
    auto it = m_fighters.find(playerId);
    if (it != m_fighters.end()) {
        auto& interpState = m_fighterInterpolation[playerId];
        auto& transforms = GetRegistry().get_components<Transform>();
        Entity fighter = it->second;
        
        if (fighter < transforms.size() && transforms[fighter].has_value()) {
            interpState.currentPos = transforms[fighter]->position;
            interpState.targetPos = targetPosition;
            interpState.interpolationTime = 0.0f;
        }
    } else {
        bool facingRight = (m_fighters.size() == 0);
        SpawnFighter(playerId, targetPosition, facingRight);
    }
}

void SecondGameClient::UpdateFighterAnimation(uint16_t playerId, uint8_t state, bool facingRight) {
    auto it = m_fighters.find(playerId);
    if (it == m_fighters.end()) return;
    
    Entity fighter = it->second;
    auto& transforms = GetRegistry().get_components<Transform>();
    auto& sprites = GetRegistry().get_components<Sprite>();
    
    if (fighter >= transforms.size() || !transforms[fighter].has_value()) return;
    if (fighter >= sprites.size() || !sprites[fighter].has_value()) return;
    
    auto& transform = transforms[fighter].value();
    float scaleX = facingRight ? 3.0f : -3.0f;
    if (transform.scale.x != scaleX) {
        transform.scale.x = scaleX;
    }
}

void SecondGameClient::InterpolateFighters(float deltaTime) {
    auto& transforms = GetRegistry().get_components<Transform>();
    
    for (auto& [playerId, interpState] : m_fighterInterpolation) {
        auto it = m_fighters.find(playerId);
        if (it == m_fighters.end()) continue;
        
        Entity fighter = it->second;
        if (fighter >= transforms.size() || !transforms[fighter].has_value()) continue;
        
        interpState.interpolationTime += deltaTime;
        float t = std::min(1.0f, interpState.interpolationTime / interpState.INTERPOLATION_DURATION);
        
        Vector2 interpolatedPos;
        interpolatedPos.x = interpState.currentPos.x + 
                           (interpState.targetPos.x - interpState.currentPos.x) * t;
        interpolatedPos.y = interpState.currentPos.y + 
                           (interpState.targetPos.y - interpState.currentPos.y) * t;
        
        transforms[fighter]->position = interpolatedPos;
    }
}

void SecondGameClient::UpdateUI() {
    m_timerText->SetText(std::to_string(m_roundTimer));
    
    m_roundText->SetText("ROUND " + std::to_string(m_currentRound));
    
    if (m_roundWins.size() >= 1) {
        auto it = m_roundWins.begin();
        m_player1WinsText->SetText("Wins: " + std::to_string(it->second));
        ++it;
        if (it != m_roundWins.end()) {
            m_player2WinsText->SetText("Wins: " + std::to_string(it->second));
        }
    }
}

void SecondGameClient::GetGameEvents(float deltaTime) {
    while (true) {
        Event e = GetNetwork()->PopEvent();
        
        if (e.type == EventType::UNKNOWN) {
            break;
        }
        
        if (e.type == EventType::GAME_END) {
            const auto& gameEnd = std::get<GAME_END>(e.data);
            
            std::cout << "[CLIENT] GAME_END received!" << std::endl;
            
            GetSceneData().Set<bool>("victory", gameEnd.victory == 1);
            
            std::vector<std::tuple<uint16_t, uint32_t, uint8_t>> scores;
            for (const auto& s : gameEnd.scores) {
                scores.push_back({s.playerId, s.score, s.rank});
            }
            GetSceneData().Set("scores", scores);
            
            ChangeScene("gameover");
            return;
        }
        
        std::visit([&](auto&& payload) {
            using T = std::decay_t<decltype(payload)>;
            
            if constexpr (std::is_same_v<T, GAME_STATE>) {
                UpdateFighters(payload.players, deltaTime);
                
                if (GetSceneData().Has("round_number")) {
                    m_currentRound = GetSceneData().Get<int>("round_number", 1);
                }
                if (GetSceneData().Has("round_timer")) {
                    m_roundTimer = GetSceneData().Get<int>("round_timer", 99);
                }
            }
        }, e.data);
    }
}

void SecondGameClient::UpdateFighters(const std::vector<GAME_STATE::PlayerState>& fighters, float deltaTime) {
    std::unordered_set<uint16_t> activeFighters;
    
    for (const auto& deltaState : fighters) {
        uint16_t playerId = deltaState.playerId;
        activeFighters.insert(playerId);
        
        if (deltaState.mask & M_DELETE) {
            RemoveFighter(playerId);
            m_fighterStates.erase(playerId);
            continue;
        }
        
        auto& fullState = m_fighterStates[playerId];
        
        if (deltaState.mask & M_POS_X) fullState.posX = deltaState.posX;
        if (deltaState.mask & M_POS_Y) fullState.posY = deltaState.posY;
        if (deltaState.mask & M_HP) fullState.hp = deltaState.hp;
        if (deltaState.mask & M_STATE) fullState.state = deltaState.state;
        if (deltaState.mask & M_SPRITE) fullState.sprite = deltaState.sprite;
        if (deltaState.mask & M_SCORE) fullState.score = deltaState.score;
        fullState.playerId = playerId;
        
        UpdateFighterPosition(playerId, {fullState.posX, fullState.posY});
        
        m_roundWins[playerId] = fullState.score;
        
        auto it = m_fighters.find(playerId);
        if (it != m_fighters.end()) {
            size_t fighterIndex = std::distance(m_fighters.begin(), it);
            if (fighterIndex == 0) {
                m_player1HealthText->SetText("P1 HP: " + std::to_string(static_cast<int>(fullState.hp)));
            } else {
                m_player2HealthText->SetText("P2 HP: " + std::to_string(static_cast<int>(fullState.hp)));
            }
        }
    }
    
    std::vector<uint16_t> toRemove;
    for (const auto& [playerId, entity] : m_fighters) {
        if (activeFighters.find(playerId) == activeFighters.end()) {
            toRemove.push_back(playerId);
        }
    }
    
    for (uint16_t playerId : toRemove) {
        RemoveFighter(playerId);
    }
}

std::string SecondGameClient::GetFighterTexture(uint8_t fighterType) const {
    return "fighter1";
}

std::string SecondGameClient::GetFighterAnimation(uint8_t state) const {
    switch (state) {
        default: return "fighter1_idle";
    }
}

extern "C" {
    Scene* CreateScene() {
        return new SecondGameClient();
    }
}
