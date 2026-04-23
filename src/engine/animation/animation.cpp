
#include "animation/animation.h"

// ============================================================
// Construction / destruction
// ============================================================

Animation::Animation(std::string name, bool loop) : m_name(std::move(name)), m_loop(loop) {}

Animation::~Animation() {}

// ============================================================
// Public Methods
// ============================================================

void Animation::buildFromSheet(int cols, int rows, int row, int firstCol, int frameCount,
                               float duration)
{
    m_frameDuration = duration;
    m_frames.clear();
    m_frames.reserve(frameCount);

    const float fw = 1.0f / static_cast<float>(cols);
    const float fh = 1.0f / static_cast<float>(rows);

    for (int i = 0; i < frameCount; ++i)
    {
        int index = firstCol + i;

        int col = index % cols;
        int r   = row + (index / cols);

        if (r >= rows)
            break;

        m_frames.push_back({
            col * fw,
            r * fh,
            (col + 1) * fw,
            (r + 1) * fh
        });
    }
}