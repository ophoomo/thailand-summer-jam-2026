#ifndef D02DF287_FFDA_4125_8E83_CC541F1A1C73
#define D02DF287_FFDA_4125_8E83_CC541F1A1C73

#include <string>
#include <vector>

struct AnimFrame
{
    float u0, v0, u1, v1;
};

class Animation
{
  public:
    Animation(std::string name, bool loop = true);
    ~Animation();

    void buildFromSheet(int cols, int rows, int row, int firstCol, int frameCount,
                        float duration = 0.1f);

    const std::string &name() const { return m_name; }
    bool loop() const { return m_loop; }
    int frameCount() const { return static_cast<int>(m_frames.size()); }
    float frameDuration() const { return m_frameDuration; }
    const AnimFrame &frameAt(int idx) const { return m_frames[idx]; }

  private:
    std::string m_name;
    bool m_loop{false};
    float m_frameDuration{0.1f};
    std::vector<AnimFrame> m_frames;
};

#endif /* D02DF287_FFDA_4125_8E83_CC541F1A1C73 */
