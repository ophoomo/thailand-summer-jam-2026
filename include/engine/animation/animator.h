#ifndef BC0AEE0F_A35D_4176_AB8B_8752AF407336
#define BC0AEE0F_A35D_4176_AB8B_8752AF407336

#include "animation/animation.h"
#include <unordered_map>

class Animator
{
  public:
    Animator();
    ~Animator();

    void addAnimation(Animation anim);

    void play(const std::string &name);
    void restart(const std::string &name);
    void onUpdate(float dt);

    const std::string &currentName() const
    {
        return m_current;
    }
    bool isPlaying() const
    {
        return !m_current.empty();
    }
    bool isFinished() const
    {
        return m_finished;
    }

    // Returns nullptr if no animation is playing or animation has no frames
    const AnimFrame *currentFrame() const;

  private:
    std::unordered_map<std::string, Animation> m_anims;
    std::string m_current;
    int m_frame = 0;
    float m_elapsed = 0.f;
    bool m_finished = false;
};

#endif /* BC0AEE0F_A35D_4176_AB8B_8752AF407336 */
