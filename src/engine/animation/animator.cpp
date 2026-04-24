
#include "animation/animator.h"

// ============================================================
// Construction / destruction
// ============================================================

Animator::Animator() {}

Animator::~Animator() {}

// ============================================================
// Public Methods
// ============================================================

void Animator::addAnimation(Animation anim)
{
    m_anims.emplace(anim.name(), std::move(anim));
}

void Animator::play(const std::string &name)
{
    if (m_current == name)
        return;
    m_current = name;
    m_frame = 0;
    m_elapsed = 0.f;
    m_finished = false;
}

void Animator::restart(const std::string &name)
{
    m_current = name;
    m_frame = 0;
    m_elapsed = 0.f;
    m_finished = false;
}

void Animator::onUpdate(float dt)
{
    if (m_current.empty() || m_finished)
        return;

    auto it = m_anims.find(m_current);
    if (it == m_anims.end())
        return;

    const Animation &anim = it->second;
    if (anim.frameCount() == 0)
        return;

    m_elapsed += dt;
    while (m_elapsed >= anim.frameDuration()) {
        m_elapsed -= anim.frameDuration();
        ++m_frame;
        if (m_frame >= anim.frameCount()) {
            if (anim.loop()) {
                m_frame = 0;
            } else {
                m_frame = anim.frameCount() - 1;
                m_finished = true;
                break;
            }
        }
    }
}

const AnimFrame *Animator::currentFrame() const
{
    if (m_current.empty())
        return nullptr;
    auto it = m_anims.find(m_current);
    if (it == m_anims.end())
        return nullptr;
    const Animation &anim = it->second;
    if (anim.frameCount() == 0)
        return nullptr;
    return &anim.frameAt(m_frame);
}
