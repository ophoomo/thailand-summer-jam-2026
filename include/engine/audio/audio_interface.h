#ifndef AB2E3461_59BA_4C7C_A52E_AC6FB6A7B71A
#define AB2E3461_59BA_4C7C_A52E_AC6FB6A7B71A

#include <string>
#include <vector>

class AudioInterface
{
  public:
    AudioInterface() = default;
    ~AudioInterface() = default;

    virtual bool load(const std::string &name, int channels, int samples, int sample_rate,
                      short *data) = 0;

    // One-shot sound effect (up to SFX_POOL simultaneous sources).
    virtual void play_sfx(const std::string &name, float volume = 1.f) = 0;
    virtual void stop_all_sfx() = 0;

    // 3D positional sound effect — world-space position, mono buffers only.
    virtual void play_sfx_3d(const std::string &name, float x, float y, float z,
                             float volume = 1.f) = 0;

    // Listener state for 3D audio.
    virtual void set_listener_position(float x, float y, float z) = 0;
    virtual void set_listener_orientation(float fx, float fy, float fz, float ux, float uy,
                                          float uz) = 0;
    virtual void set_listener_velocity(float x, float y, float z) = 0;

    // Background music — one track at a time, loops by default.
    virtual void play_bgm(const std::string &name, bool loop = true, float volume = 1.f) = 0;
    virtual void stop_bgm(float fade_time = 0) = 0;
    virtual void pause_bgm() = 0;
    virtual void resume_bgm() = 0;

    // Volume control (0.0 – 1.0).
    virtual void set_master_volume(float v) = 0;
    virtual void set_sfx_volume(float v) = 0;
    virtual void set_bgm_volume(float v) = 0;
    virtual void set_bgm_fade_gain(float v) = 0;
    virtual float get_master_volume() const = 0;
    virtual float get_sfx_volume() const = 0;
    virtual float get_bgm_volume() const = 0;

    struct AudioFade
    {
        bool active = false;
        float start = 1.f;
        float target = 1.f;
        float duration = 0.f;
        float elapsed = 0.f;
    };
    virtual void fade_bgm(float target, float duration) = 0;
    virtual void update(float dt) = 0;

    // Debug queries
    virtual std::vector<std::string> get_loaded_names() const = 0;
    virtual std::string get_bgm_name() const = 0;
    virtual std::string get_bgm_state() const = 0; // "playing"|"paused"|"stopped"

    struct SoundInfo
    {
        int channels = 0;
        int sample_rate = 0;
        float duration_sec = 0.f;
        bool valid = false;
    };
    virtual SoundInfo get_sound_info(const std::string &name) const = 0;
    virtual float get_bgm_position() const = 0;
    virtual float get_bgm_duration() const = 0;
    virtual int get_active_sfx_count() const = 0;
    virtual int get_active_sfx_3d_count() const = 0;

    struct SfxSourceInfo
    {
        int index = 0;
        bool is_3d = false;
        std::string sound_name;
        float gain = 0.f;
        float pos[3] = {};
    };
    virtual std::vector<SfxSourceInfo> get_sfx_sources_info() const = 0;
    virtual void stop_sfx_source(int index, bool is_3d) = 0;
};

#endif /* AB2E3461_59BA_4C7C_A52E_AC6FB6A7B71A */
