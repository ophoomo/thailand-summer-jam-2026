#ifndef F7620651_E3DD_4DD2_90C5_A70F09ED452D
#define F7620651_E3DD_4DD2_90C5_A70F09ED452D

#include "core/scene_manager.h"

class DebugScene
{
  public:
    DebugScene(SceneManager *scenes);
    ~DebugScene();

    void onDraw();

  private:
    SceneManager *m_scenes;
};

#endif /* F7620651_E3DD_4DD2_90C5_A70F09ED452D */
