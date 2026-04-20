#ifndef D617456A_C578_415C_8C17_F0651099859C
#define D617456A_C578_415C_8C17_F0651099859C

#include "entt/signal/fwd.hpp"
#include "ui/ui_system.h"
#include <string>

class AudioInterface;

class ScriptManager
{
  public:
    ScriptManager();
    ~ScriptManager();

    // Non-copyable (owns lua_State*)
    ScriptManager(const ScriptManager &) = delete;
    ScriptManager &operator=(const ScriptManager &) = delete;

    void BindScene(entt::dispatcher *dispatcher, std::string current_scene_name);
    void BindUI(UISystem *ui);
    void BindApp(bool *quit_flag);
    void BindInput(float *mouse_x, float *mouse_y, bool *mouse_clicked, bool *mouse_held);
    void BindApp(entt::dispatcher *dispatcher);
    void BindAudio(AudioInterface *audio);
    void BindLocalization();

    bool RunScript(const std::string name, const std::string &code);

    bool CallVoid(const std::string &fn_name);
    bool CallWithFloat(const std::string &fn_name, float arg);

    [[nodiscard]] lua_State *State() const
    {
        return this->m_L;
    }

  private:
    lua_State *m_L = nullptr;
};

#endif /* D617456A_C578_415C_8C17_F0651099859C */
