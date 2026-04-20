
#include <stdexcept>
#include "core/scene_manager.h"
#include "core/scene.h"
#include "entt/entt.hpp"
#include "utils/logger.h"
#include <memory>

// ============================================================
// Construction / destruction
// ============================================================

SceneManager::SceneManager(std::shared_ptr<entt::dispatcher> dispatcher,
                           std::shared_ptr<OxRenderer> renderer, std::shared_ptr<Window> window)
{
    this->m_dispatcher = dispatcher;
    this->m_renderer = renderer;
    this->m_window = window;
}

SceneManager::~SceneManager() {}

// ============================================================
// Public Methods
// ============================================================

void SceneManager::onEvent()
{
    this->m_dispatcher->sink<SceneEvent>().connect<&SceneManager::onSceneEvent>(this);
}

void SceneManager::onUpdate(double deltaTime)
{
    this->onChangeScene();

    if (this->m_transState == TransitionState::FadeOut) {
        this->m_fadeAlpha += static_cast<float>(deltaTime) / this->m_fadeDuration;
        if (this->m_fadeAlpha >= 1.0f) {
            this->m_fadeAlpha = 1.0f;
            if (this->m_pendingScene.has_value()) {
                this->next = this->m_pendingScene;
                this->m_pendingScene.reset();
                this->onChangeScene();
            }
            this->m_transState = TransitionState::FadeIn;
        }
    } else if (this->m_transState == TransitionState::FadeIn) {
        this->m_fadeAlpha -= static_cast<float>(deltaTime) / this->m_fadeDuration;
        if (this->m_fadeAlpha <= 0.0f) {
            this->m_fadeAlpha = 0.0f;
            this->m_transState = TransitionState::None;
        }
    }

    this->currentScene->onUpdate(deltaTime);
}

void SceneManager::onDraw()
{
    this->currentScene->onDraw();
    if (this->m_transState != TransitionState::None) {
        float w = static_cast<float>(this->m_window->getWidth());
        float h = static_cast<float>(this->m_window->getHeight());
        uint8_t alpha = static_cast<uint8_t>(std::clamp(this->m_fadeAlpha, 0.0f, 1.0f) * 255.0f);
        this->m_renderer->oxDrawRectangle(0.0f, 0.0f, w, h, Color(0, 0, 0, alpha), 999);
    }
}

void SceneManager::addScene(std::shared_ptr<Scene> scene, const std::string name)
{
    if (this->m_scenes.count(name)) {
        auto error = std::format("[SceneManager] addScene - Duplicate key: '{}'", name);
        LOG_CORE_ERROR(error);
        throw std::runtime_error(error);
    }
    this->m_scenes.insert({name, scene});
}

void SceneManager::change(const std::string name)
{
    if (this->m_scenes.count(name) == 0) {
        auto error = std::format("[SceneManager] change - Scene not found: '{}'", name);
        LOG_CORE_ERROR(error);
        throw std::runtime_error(error);
    }

    if (!this->currentScene) {
        this->next = name;
        return;
    }

    this->m_pendingScene = name;
    this->m_fadeAlpha = 0.0f;
    this->m_transState = TransitionState::FadeOut;

    if (this->currentScene && this->currentScene->m_audio)
        this->currentScene->m_audio->fade_bgm(0.0f, this->m_fadeDuration);
}

void SceneManager::onChangeScene()
{
    if (this->next.has_value()) {
        if (this->currentScene) {
            this->m_renderer->waitIdle();
            if (this->currentScene->m_audio)
                this->currentScene->m_audio->stop_all_sfx();
            this->currentScene->onExit();
        }

        this->previous = this->current;
        this->current = this->next.value();
        this->currentScene = this->m_scenes.at(this->current);
        this->next.reset();

        this->currentScene->onEnter();
    }
}

std::vector<std::string> SceneManager::SceneNames() const
{
    std::vector<std::string> names;
    names.reserve(this->m_scenes.size());
    for (const auto &[k, _] : this->m_scenes)
        names.push_back(k);
    return names;
}

void SceneManager::onSceneEvent(const SceneEvent &event)
{
    this->change(event.scene);
}
