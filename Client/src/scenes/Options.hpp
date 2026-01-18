#pragma once
#include <SDL2/SDL.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "Helpers/EntityHelper.hpp"
#include "audio/AudioSubsystem.hpp"
#include "engine/GameEngine.hpp"
#include "input/KeyBindings.hpp"
#include "scene/Scene.hpp"
#include "scene/SceneManager.hpp"
#include "settings/PlayerSettings.hpp"
#include "ui/UIButton.hpp"
#include "ui/UIManager.hpp"
#include "ui/UIText.hpp"

#ifdef _WIN32
#ifdef PlaySound
#undef PlaySound
#endif
#endif

class OptionsScene : public Scene {
 private:
  bool m_isInitialized;
  bool m_waitingForKey;
  GameAction m_actionToRebind;

  UIText* m_promptText;
  UIText* m_skinTitle;
  UIText* m_skinNameText;
  UIButton* m_prevSkinButton;
  UIButton* m_nextSkinButton;
  Entity m_previewPlayer;
  Entity m_background;

  struct KeyBindingRow {
    UIText* actionLabel;
    UIButton* keyButton;
    UIButton* clearButton;
    GameAction action;
  };

  std::vector<KeyBindingRow> m_bindingRows;
  UIButton* m_resetButton;
  UIButton* m_saveButton;
  UIButton* m_backButton;

  UIButton* m_musicMuteButton;
  UIButton* m_sfxMuteButton;

  PlayerSkin m_currentSkin;
  PlayerSettings m_settings;

  bool m_sfxMute;
  bool m_musicMute;

 public:
  OptionsScene()
      : m_isInitialized(false),
        m_waitingForKey(false),
        m_actionToRebind(GameAction::NONE),
        m_promptText(nullptr),
        m_skinTitle(nullptr),
        m_skinNameText(nullptr),
        m_prevSkinButton(nullptr),
        m_nextSkinButton(nullptr),
        m_previewPlayer(0),
        m_background(1),
        m_resetButton(nullptr),
        m_saveButton(nullptr),
        m_backButton(nullptr),
        m_musicMuteButton(nullptr),
        m_sfxMuteButton(nullptr),
        m_sfxMute(false),
        m_musicMute(false),
        m_currentSkin(PlayerSkin::BLUE) {
    m_name = "options";
  }

  void OnEnter() override {
    std::cout << "\n=== ENTERING OPTIONS SCENE ===" << std::endl;

    try {
      m_isInitialized = false;
      m_waitingForKey = false;
      m_bindingRows.clear();

      m_currentSkin = m_settings.GetSelectedSkin();

      auto* titleText = GetUI()->AddElement<UIText>(
          400, 30, "OPTIONS", "", 40, SDL_Color{255, 255, 255, 255});
      titleText->SetAlignment(TextAlign::Center);
      titleText->SetLayer(10);

      // === SKIN SELECTION SECTION ===
      m_skinTitle = GetUI()->AddElement<UIText>(400, 80, "PLAYER SKIN", "", 28,
                                                SDL_Color{200, 200, 200, 255});
      m_skinTitle->SetAlignment(TextAlign::Center);
      m_skinTitle->SetLayer(10);

      m_prevSkinButton = GetUI()->AddElement<UIButton>(250, 120, 50, 50, "<");
      m_prevSkinButton->SetColors({70, 70, 100, 255}, {90, 90, 120, 255},
                                  {50, 50, 80, 255});
      m_prevSkinButton->SetFontSize(32);
      m_prevSkinButton->SetLayer(10);
      m_prevSkinButton->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        ChangeSkin(PlayerSettings::GetPreviousSkin(m_currentSkin));
      });

      m_skinNameText = GetUI()->AddElement<UIText>(
          400, 140, PlayerSettings::GetSkinName(m_currentSkin), "", 24,
          SDL_Color{200, 200, 200, 255});
      m_skinNameText->SetAlignment(TextAlign::Center);
      m_skinNameText->SetLayer(10);

      m_nextSkinButton = GetUI()->AddElement<UIButton>(500, 120, 50, 50, ">");
      m_nextSkinButton->SetColors({70, 70, 100, 255}, {90, 90, 120, 255},
                                  {50, 50, 80, 255});
      m_nextSkinButton->SetFontSize(32);
      m_nextSkinButton->SetLayer(10);
      m_nextSkinButton->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        ChangeSkin(PlayerSettings::GetNextSkin(m_currentSkin));
      });

      m_background = CreateSprite(GetRegistry(), "background", {400, 300}, -10);
      // AnimationManager& animations = GetAnimations();
      m_previewPlayer = CreateAnimatedSprite(
          GetRegistry(),
          GetRendering()->GetAnimation(
              PlayerSettings::GetSkinAnimation(m_currentSkin)),
          "player", {400, 220}, PlayerSettings::GetSkinAnimation(m_currentSkin),
          0);
      auto& transform =
          GetRegistry().get_components<Transform>()[m_previewPlayer];
      if (transform) {
        transform->scale = {3.0f, 3.0f};
      }

      // === KEY BINDINGS SECTION ===
      auto* bindingsTitle = GetUI()->AddElement<UIText>(
          400, 280, "KEY BINDINGS", "", 28, SDL_Color{200, 200, 200, 255});
      bindingsTitle->SetAlignment(TextAlign::Center);
      bindingsTitle->SetLayer(10);

      m_promptText = GetUI()->AddElement<UIText>(
          400, 320, "Press any key...", "", 20, SDL_Color{255, 255, 100, 255});
      m_promptText->SetAlignment(TextAlign::Center);
      m_promptText->SetLayer(10);
      m_promptText->SetVisible(false);

      std::vector<GameAction> actions = {
          GameAction::MOVE_UP, GameAction::MOVE_DOWN, GameAction::MOVE_LEFT,
          GameAction::MOVE_RIGHT, GameAction::FIRE};

      int yPos = 350;
      for (GameAction action : actions) {
        CreateBindingRow(action, yPos);
        yPos += 45;
      }

      m_resetButton =
          GetUI()->AddElement<UIButton>(150, 560, 140, 40, "Reset Keys");
      m_resetButton->SetColors({150, 50, 50, 255}, {170, 70, 70, 255},
                               {130, 30, 30, 255});
      m_resetButton->SetFontSize(18);
      m_resetButton->SetLayer(10);
      m_resetButton->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        ResetToDefaults();
      });

      m_saveButton = GetUI()->AddElement<UIButton>(330, 560, 140, 40, "Save");
      m_saveButton->SetColors({50, 150, 50, 255}, {70, 170, 70, 255},
                              {30, 130, 30, 255});
      m_saveButton->SetFontSize(18);
      m_saveButton->SetLayer(10);
      m_saveButton->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        SaveSettings();
      });

      m_backButton = GetUI()->AddElement<UIButton>(510, 560, 140, 40, "Back");
      m_backButton->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                              {80, 80, 80, 255});
      m_backButton->SetFontSize(18);
      m_backButton->SetLayer(10);
      m_backButton->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        ChangeScene("menu");
      });

      m_sfxMuteButton =
          GetUI()->AddElement<UIButton>(510, 10, 140, 40, "Mute SFX");
      m_sfxMuteButton->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                 {80, 80, 80, 255});
      m_sfxMuteButton->SetFontSize(18);
      m_sfxMuteButton->SetLayer(10);
      m_sfxMuteButton->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        m_sfxMute = !m_sfxMute;
        GetAudio()->MuteSFX(m_sfxMute);
      });

      m_musicMuteButton =
          GetUI()->AddElement<UIButton>(660, 10, 140, 40, "Mute Music");
      m_musicMuteButton->SetColors({100, 100, 100, 255}, {150, 150, 150, 255},
                                   {80, 80, 80, 255});
      m_musicMuteButton->SetFontSize(18);
      m_musicMuteButton->SetLayer(10);
      m_musicMuteButton->SetOnClick([this]() {
        GetAudio()->PlaySound("button");
        m_musicMute = !m_musicMute;
        GetAudio()->MuteMusic(m_musicMute);
      });

      m_isInitialized = true;
      std::cout << "Options scene initialized" << std::endl;
    } catch (const std::exception& e) {
      std::cerr << "ERROR in OptionsScene::OnEnter: " << e.what() << std::endl;
      m_isInitialized = false;
    }
  }

  void OnExit() override {
    std::cout << "\n=== EXITING OPTIONS SCENE ===" << std::endl;
    m_isInitialized = false;
    m_waitingForKey = false;
    m_bindingRows.clear();
    GetUI()->Clear();
  }

  void Update(float deltaTime) override {
    if (!m_isInitialized) return;
    // GetUI()->Update(deltaTime);
  }

  void Render() override {
    if (!m_isInitialized) return;

    // SDL_Renderer* renderer = GetRender();
    // SDL_SetRenderDrawColor(renderer, 30, 30, 40, 255);
    // SDL_RenderClear(renderer);

    // RenderSpritesLayered();

    // GetUI().Render();
  }

  void HandleEvent(SDL_Event& event) override {
    if (!m_isInitialized) return;

    if (m_waitingForKey && event.type == SDL_KEYDOWN) {
      HandleKeyRebind(event.key.keysym.scancode);
      return;
    }

    if (GetUI()->HandleEvent(event)) return;

    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
      if (m_waitingForKey) {
        CancelRebind();
      } else {
        ChangeScene("menu");
      }
    }
  }

 private:
  void ChangeSkin(PlayerSkin newSkin) {
    m_currentSkin = newSkin;
    m_skinNameText->SetText(PlayerSettings::GetSkinName(newSkin));

    auto& animations = GetRegistry().get_components<Animation>();
    if (m_previewPlayer < animations.size() &&
        animations[m_previewPlayer].has_value()) {
      animations[m_previewPlayer]->animationKey =
          PlayerSettings::GetSkinAnimation(newSkin);
      animations[m_previewPlayer]->currentFrame = 0;
      animations[m_previewPlayer]->currentTime = 0;
    }

    std::cout << "Changed skin to: " << PlayerSettings::GetSkinName(newSkin)
              << std::endl;
  }

  void SaveSettings() {
    std::cout << "Saving settings..." << std::endl;

    m_settings.SetSelectedSkin(m_currentSkin);
    m_settings.SaveToFile();

    GetInput()->SaveKeyBindings();

    m_saveButton->SetText("Saved!");
  }

  void CreateBindingRow(GameAction action, int yPos) {
    KeyBindings& bindings = GetInput()->GetKeyBindings();
    KeyBindingRow row;
    row.action = action;

    row.actionLabel =
        GetUI()->AddElement<UIText>(150, yPos, bindings.GetActionName(action),
                                    "", 18, SDL_Color{200, 200, 200, 255});
    row.actionLabel->SetLayer(10);

    std::string keyName = bindings.GetKeyNameForAction(action);
    row.keyButton =
        GetUI()->AddElement<UIButton>(350, yPos - 5, 150, 35, keyName);
    row.keyButton->SetColors({70, 70, 100, 255}, {90, 90, 120, 255},
                             {50, 50, 80, 255});
    row.keyButton->SetFontSize(16);
    row.keyButton->SetLayer(10);
    row.keyButton->SetOnClick([this, action]() { StartRebind(action); });

    row.clearButton =
        GetUI()->AddElement<UIButton>(520, yPos - 5, 60, 35, "Clear");
    row.clearButton->SetColors({100, 50, 50, 255}, {120, 70, 70, 255},
                               {80, 30, 30, 255});
    row.clearButton->SetFontSize(14);
    row.clearButton->SetLayer(10);
    row.clearButton->SetOnClick([this, action]() { ClearBinding(action); });

    m_bindingRows.push_back(row);
  }

  void StartRebind(GameAction action) {
    m_waitingForKey = true;
    m_actionToRebind = action;
    m_promptText->SetText("Press key for " +
                          GetInput()->GetKeyBindings().GetActionName(action));
    m_promptText->SetVisible(true);

    for (auto& row : m_bindingRows) {
      row.keyButton->SetEnabled(false);
      row.clearButton->SetEnabled(false);
    }
    m_resetButton->SetEnabled(false);
    m_saveButton->SetEnabled(false);
    m_backButton->SetEnabled(false);
    m_prevSkinButton->SetEnabled(false);
    m_nextSkinButton->SetEnabled(false);
  }

  void HandleKeyRebind(SDL_Scancode key) {
    if (key == SDL_SCANCODE_ESCAPE) {
      CancelRebind();
      return;
    }

    GetInput()->GetKeyBindings().UnbindAction(m_actionToRebind);
    GetInput()->GetKeyBindings().BindKey(m_actionToRebind, key);
    UpdateBindingDisplay(m_actionToRebind);
    EndRebind();
  }

  void CancelRebind() { EndRebind(); }

  void EndRebind() {
    m_waitingForKey = false;
    m_actionToRebind = GameAction::NONE;
    m_promptText->SetVisible(false);

    for (auto& row : m_bindingRows) {
      row.keyButton->SetEnabled(true);
      row.clearButton->SetEnabled(true);
    }
    m_resetButton->SetEnabled(true);
    m_saveButton->SetEnabled(true);
    m_backButton->SetEnabled(true);
    m_prevSkinButton->SetEnabled(true);
    m_nextSkinButton->SetEnabled(true);
  }

  void UpdateBindingDisplay(GameAction action) {
    KeyBindings& bindings = GetInput()->GetKeyBindings();
    for (auto& row : m_bindingRows) {
      if (row.action == action) {
        row.keyButton->SetText(bindings.GetKeyNameForAction(action));
        break;
      }
    }
  }

  void ClearBinding(GameAction action) {
    GetInput()->GetKeyBindings().UnbindAction(action);
    UpdateBindingDisplay(action);
  }

  void ResetToDefaults() {
    GetInput()->ResetKeyBindings();
    for (auto& row : m_bindingRows) {
      UpdateBindingDisplay(row.action);
    }
  }

  std::unordered_map<uint16_t, Entity> GetPlayers() override {
    return std::unordered_map<uint16_t, Entity>();
  }
};


#ifdef _WIN32
extern "C" {
  __declspec(dllexport) Scene* CreateScene();
}
#else
extern "C" {
    Scene* CreateScene();
}
#endif
