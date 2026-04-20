#ifndef D09E11BA_F4EF_480F_AAE2_EE44D1C8BC58
#define D09E11BA_F4EF_480F_AAE2_EE44D1C8BC58

#include "audio/audio_interface.h"

class DebugAudio
{
  public:
    explicit DebugAudio(AudioInterface *audio);
    ~DebugAudio() = default;

    void onDraw();

  private:
    AudioInterface *m_audio;
    char m_filter[64] = {};
    float m_test_3d_pos[3] = {5.f, 0.f, 0.f};
};

#endif /* D09E11BA_F4EF_480F_AAE2_EE44D1C8BC58 */
