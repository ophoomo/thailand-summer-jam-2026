#ifndef BAD10DA8_FBF2_4520_93FC_067A8A9669F0
#define BAD10DA8_FBF2_4520_93FC_067A8A9669F0

#include "assets/assets_interface.h"
#include "audio/audio_interface.h"
#include "entt/entt.hpp"
#include "renderer/ox_renderer.h"
#include "ui/cursor_ui.h"
#include <memory>

class Scene
{
  public:
    Scene(std::shared_ptr<entt::dispatcher> dispatcher, std::shared_ptr<OxRenderer> renderer,
          std::shared_ptr<AssetsInterface> assets, std::shared_ptr<AudioInterface> audio,
          std::shared_ptr<CursorUI> cursor)
        : m_dispatcher(dispatcher), m_renderer(renderer), m_assets(assets), m_audio(audio),
          m_cursor(cursor)
    {
    }
    ~Scene() = default;

    virtual void onEnter() = 0;
    virtual void onDraw() = 0;
    virtual void onUpdate(double deltaTime) = 0;
    virtual void onExit() = 0;

    std::shared_ptr<entt::dispatcher> m_dispatcher;
    std::shared_ptr<OxRenderer> m_renderer;
    std::shared_ptr<AssetsInterface> m_assets;
    std::shared_ptr<AudioInterface> m_audio;
    std::shared_ptr<CursorUI> m_cursor;
};

#endif /* BAD10DA8_FBF2_4520_93FC_067A8A9669F0 */
