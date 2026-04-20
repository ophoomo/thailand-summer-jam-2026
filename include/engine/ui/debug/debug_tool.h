#ifndef D7CE06DB_2AC4_437D_9110_92E1C09AE8E5
#define D7CE06DB_2AC4_437D_9110_92E1C09AE8E5

#include "core/scene_manager.h"
#include "renderer/ox_renderer.h"
#include "ui/debug/debug_audio.h"
#include "ui/debug/debug_scene.h"
#include <memory>

class DebugTools
{
  public:
    DebugTools(OxRenderer *renderer, SceneManager *scenes, AudioInterface *audio);
    ~DebugTools();

    void onDraw(double dt);

  private:
    void apply_style();

    std::unique_ptr<DebugScene> m_panel_scene;
    std::unique_ptr<DebugAudio> m_panel_audio;

    bool m_show_scene = false;
    bool m_show_audio = false;
};

#endif /* D7CE06DB_2AC4_437D_9110_92E1C09AE8E5 */
