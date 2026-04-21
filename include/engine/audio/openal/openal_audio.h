#ifndef C6EAAA2B_1E69_412C_8F65_85FD8D68C2FC
#define C6EAAA2B_1E69_412C_8F65_85FD8D68C2FC

#include "al.h"
#include "alc.h"
#include "audio/audio_interface.h"
#include <algorithm>
#include <unordered_map>

class OpenALAudio : public AudioInterface
{
  public:
    OpenALAudio();
    ~OpenALAudio();

    bool load(const std::string &name, int channels, int samples, int sample_rate,
              short *data) override;
    void unload(const std::string &name) override;

    void play_sfx(const std::string &name, float volume = 1.f) override;
    void stop_all_sfx() override;
    void play_sfx_3d(const std::string &name, float x, float y, float z,
                     float volume = 1.f) override;

    void set_listener_position(float x, float y, float z) override;
    void set_listener_orientation(float fx, float fy, float fz, float ux, float uy,
                                  float uz) override;
    void set_listener_velocity(float x, float y, float z) override;

    void play_bgm(const std::string &name, bool loop = true, float volume = 1.f) override;
    void stop_bgm(float fade_time = 0) override;
    void pause_bgm() override;
    void resume_bgm() override;

    void set_master_volume(float v) override;
    void set_sfx_volume(float v) override;
    void set_bgm_volume(float v) override;
    void set_bgm_fade_gain(float v) override;
    float get_master_volume() const override
    {
        return this->m_master_vol;
    }
    float get_sfx_volume() const override
    {
        return this->m_sfx_vol;
    }
    float get_bgm_volume() const override
    {
        return this->m_bgm_vol;
    }

    std::vector<std::string> get_loaded_names() const override;
    std::string get_bgm_name() const override
    {
        return this->m_bgm_current;
    }
    std::string get_bgm_state() const override;

    SoundInfo get_sound_info(const std::string &name) const override;
    float get_bgm_position() const override;
    float get_bgm_duration() const override;
    int get_active_sfx_count() const override;
    int get_active_sfx_3d_count() const override;
    std::vector<SfxSourceInfo> get_sfx_sources_info() const override;
    void stop_sfx_source(int index, bool is_3d) override;

    void fade_bgm(float target, float duration) override;
    void update(float dt) override;

  private:
    bool is_valid() const
    {
        return this->m_context != nullptr;
    }
    int next_sfx_source();
    int next_sfx_3d_source();
    void apply_bgm_volume();
    void reapply_sfx_gains();
    void reapply_sfx_3d_gains();

    ALCdevice *m_device = nullptr;
    ALCcontext *m_context = nullptr;

    std::unordered_map<std::string, ALuint> m_buffers;

    // 2D (relative) SFX pool — UI sounds, music stingers, etc.
    static constexpr int SFX_POOL = 16;
    ALuint m_sfx_sources[SFX_POOL] = {};
    float m_sfx_individual_vols[SFX_POOL] = {};
    int m_sfx_next = 0;

    // 3D (world-space) SFX pool — positional game sounds.
    static constexpr int SFX_3D_POOL = 8;
    ALuint m_sfx_3d_sources[SFX_3D_POOL] = {};
    float m_sfx_3d_individual_vols[SFX_3D_POOL] = {};
    int m_sfx_3d_next = 0;

    ALuint m_bgm_source = 0;
    std::string m_bgm_current;
    float m_bgm_individual_vol = 1.f;

    float m_master_vol = 1.f;
    float m_sfx_vol = 1.f;
    float m_bgm_vol = 1.f;

    AudioFade m_bgm_fade;
    bool m_bgm_stop_after_fade = false;
    float m_bgm_fade_gain = 1.0f;
};

#endif /* C6EAAA2B_1E69_412C_8F65_85FD8D68C2FC */
