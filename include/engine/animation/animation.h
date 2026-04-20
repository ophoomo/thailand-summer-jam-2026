#ifndef D02DF287_FFDA_4125_8E83_CC541F1A1C73
#define D02DF287_FFDA_4125_8E83_CC541F1A1C73

#include "string"
#include <vector>

struct Frame
{
};

class Animation
{
  public:
    Animation();
    ~Animation();

  private:
    std::string m_name;
    std::vector<Frame> m_frames;
    bool m_loop{false};
};

#endif /* D02DF287_FFDA_4125_8E83_CC541F1A1C73 */
