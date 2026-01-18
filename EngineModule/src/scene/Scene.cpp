#include "scene/Scene.hpp"
#include "engine/GameEngine.hpp"
#include "scene/SceneManager.hpp"
#include "rendering/RenderingSubsystem.hpp"
#include "network/NetworkSubsystem.hpp"
#include "audio/AudioSubsystem.hpp"
#include "input/InputSubsystem.hpp"

Registry& Scene::GetRegistry() {
    return m_engine->GetRegistry();
}

RenderingSubsystem* Scene::GetRendering() {
    return dynamic_cast<RenderingSubsystem*>(
        m_engine->GetSubsystem(SubsystemType::RENDERING));
}

AudioSubsystem* Scene::GetAudio() {
    return dynamic_cast<AudioSubsystem*>(
        m_engine->GetSubsystem(SubsystemType::AUDIO));
}

InputSubsystem* Scene::GetInput() {
    return dynamic_cast<InputSubsystem*>(
        m_engine->GetSubsystem(SubsystemType::INPUT));
}

NetworkSubsystem* Scene::GetNetwork() {
    return dynamic_cast<NetworkSubsystem*>(
        m_engine->GetSubsystem(SubsystemType::NETWORK));
}

UIManager* Scene::GetUI() {
    auto* rendering = GetRendering();
    return rendering ? rendering->GetUIManager() : nullptr;
}

SceneData& Scene::GetSceneData() {
    return m_sceneManager->GetSceneData();
}

void Scene::ChangeScene(const std::string& sceneName) {
    m_sceneManager->ChangeScene(sceneName);
}

void Scene::QuitGame() {
    m_engine->Stop();
}
