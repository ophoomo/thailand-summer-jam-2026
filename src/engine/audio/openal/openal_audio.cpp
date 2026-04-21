
#include "audio/openal/openal_audio.h"
#include "al.h"
#include "utils/logger.h"

// ============================================================
// Construction / destruction
// ============================================================

OpenALAudio::OpenALAudio()
{
    this->m_device = alcOpenDevice(nullptr);
    if (!this->m_device) {
        LOG_CORE_ERROR("[OpenALAudio] Failed to open audio device");
        return;
    }

    this->m_context = alcCreateContext(this->m_device, nullptr);
    if (!this->m_context || alcMakeContextCurrent(this->m_context) == ALC_FALSE) {
        LOG_CORE_ERROR("[OpenALAudio] Failed to create/activate AL context");
        alcCloseDevice(this->m_device);
        this->m_device = nullptr;
        this->m_context = nullptr;
        return;
    }

    // Distance model: gain ∝ ref / (ref + rolloff*(dist - ref)), clamped to [ref, max]
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

    ALfloat pos[] = {0.f, 0.f, 0.f};
    ALfloat vel[] = {0.f, 0.f, 0.f};
    ALfloat ori[] = {0.f, 0.f, -1.f, 0.f, 1.f, 0.f};
    alListenerfv(AL_POSITION, pos);
    alListenerfv(AL_VELOCITY, vel);
    alListenerfv(AL_ORIENTATION, ori);
    alListenerf(AL_GAIN, this->m_master_vol);

    // 2D SFX pool — source-relative, no distance attenuation.
    alGenSources(SFX_POOL, m_sfx_sources);
    for (int i = 0; i < SFX_POOL; ++i) {
        alSourcei(this->m_sfx_sources[i], AL_SOURCE_RELATIVE, AL_TRUE);
        alSourcef(this->m_sfx_sources[i], AL_ROLLOFF_FACTOR, 0.f);
        alSource3f(this->m_sfx_sources[i], AL_POSITION, 0.f, 0.f, 0.f);
    }

    // 3D SFX pool — world-space, distance attenuation enabled.
    alGenSources(SFX_3D_POOL, m_sfx_3d_sources);
    for (int i = 0; i < SFX_3D_POOL; ++i) {
        alSourcei(this->m_sfx_3d_sources[i], AL_SOURCE_RELATIVE, AL_FALSE);
        alSourcef(this->m_sfx_3d_sources[i], AL_ROLLOFF_FACTOR, 1.f);
        alSourcef(this->m_sfx_3d_sources[i], AL_REFERENCE_DISTANCE, 1.f);
        alSourcef(this->m_sfx_3d_sources[i], AL_MAX_DISTANCE, 100.f);
        alSource3f(this->m_sfx_3d_sources[i], AL_POSITION, 0.f, 0.f, 0.f);
        alSource3f(this->m_sfx_3d_sources[i], AL_VELOCITY, 0.f, 0.f, 0.f);
    }

    alGenSources(1, &this->m_bgm_source);
    alSourcei(this->m_bgm_source, AL_SOURCE_RELATIVE, AL_TRUE);
    alSourcef(this->m_bgm_source, AL_ROLLOFF_FACTOR, 0.f);
    alSource3f(this->m_bgm_source, AL_POSITION, 0.f, 0.f, 0.f);
    alSourcef(this->m_bgm_source, AL_GAIN, m_bgm_vol);

    LOG_CORE_INFO("[OpenALAudio] Initialized ({})",
                  alcGetString(this->m_device, ALC_DEVICE_SPECIFIER));
}

OpenALAudio::~OpenALAudio()
{
    if (!this->m_context)
        return;

    alSourceStop(this->m_bgm_source);
    alDeleteSources(1, &this->m_bgm_source);
    alDeleteSources(SFX_POOL, this->m_sfx_sources);
    alDeleteSources(SFX_3D_POOL, this->m_sfx_3d_sources);

    for (auto &[name, buf] : this->m_buffers)
        alDeleteBuffers(1, &buf);

    alcMakeContextCurrent(nullptr);
    alcDestroyContext(this->m_context);
    alcCloseDevice(this->m_device);
    LOG_CORE_INFO("[OpenALAudio] Shutdown");
}

// ============================================================
// Public Methods
// ============================================================

bool OpenALAudio::load(const std::string &name, int channels, int samples, int sample_rate,
                       short *data)
{
    ALenum format = (channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    ALsizei size = (ALsizei)(samples * channels * sizeof(short));

    ALuint buf = 0;
    alGenBuffers(1, &buf);
    alBufferData(buf, format, data, size, (ALsizei)sample_rate);
    free(data);

    if (alGetError() != AL_NO_ERROR) {
        LOG_CORE_ERROR("[OpenALAudio] alBufferData failed for '{}'", name);
        alDeleteBuffers(1, &buf);
        return false;
    }

    this->m_buffers[name] = buf;
    LOG_CORE_INFO("[OpenALAudio] Loaded '{}' ({}ch, {}Hz, {} samples)", name, channels, sample_rate,
                  samples);
    return true;
}

void OpenALAudio::unload(const std::string &name)
{
    auto it = this->m_buffers.find(name);
    if (it == this->m_buffers.end())
        return;

    alDeleteBuffers(1, &it->second);
    this->m_buffers.erase(it);
}

void OpenALAudio::play_sfx(const std::string &name, float volume)
{
    if (!this->is_valid())
        return;
    auto it = this->m_buffers.find(name);
    if (it == this->m_buffers.end()) {
        LOG_CORE_WARN("[OpenALAudio] play_sfx: '{}' not loaded", name);
        return;
    }

    int idx = this->next_sfx_source();
    ALuint src = this->m_sfx_sources[idx];
    this->m_sfx_individual_vols[idx] = std::clamp(volume, 0.f, 1.f);
    alSourceStop(src);
    alSourcei(src, AL_BUFFER, (ALint)it->second);
    alSourcei(src, AL_LOOPING, AL_FALSE);
    alSourcef(src, AL_GAIN, std::clamp(this->m_sfx_vol * volume, 0.f, 1.f));
    alSourcePlay(src);
}

void OpenALAudio::stop_all_sfx()
{
    if (!this->is_valid())
        return;
    for (int i = 0; i < SFX_POOL; ++i)
        alSourceStop(this->m_sfx_sources[i]);
    for (int i = 0; i < SFX_3D_POOL; ++i)
        alSourceStop(this->m_sfx_3d_sources[i]);
}

void OpenALAudio::play_sfx_3d(const std::string &name, float x, float y, float z, float volume)
{
    if (!this->is_valid())
        return;
    auto it = this->m_buffers.find(name);
    if (it == this->m_buffers.end()) {
        LOG_CORE_WARN("[OpenALAudio] play_sfx_3d: '{}' not loaded", name);
        return;
    }

    int idx = this->next_sfx_3d_source();
    ALuint src = this->m_sfx_3d_sources[idx];
    this->m_sfx_3d_individual_vols[idx] = std::clamp(volume, 0.f, 1.f);
    alSourceStop(src);
    alSourcei(src, AL_BUFFER, (ALint)it->second);
    alSourcei(src, AL_LOOPING, AL_FALSE);
    alSourcef(src, AL_GAIN, std::clamp(this->m_sfx_vol * volume, 0.f, 1.f));
    alSource3f(src, AL_POSITION, x, y, z);
    alSourcePlay(src);
}

void OpenALAudio::set_listener_position(float x, float y, float z)
{
    if (this->is_valid())
        alListener3f(AL_POSITION, x, y, z);
}

void OpenALAudio::set_listener_orientation(float fx, float fy, float fz, float ux, float uy,
                                           float uz)
{
    if (!this->is_valid())
        return;
    ALfloat ori[] = {fx, fy, fz, ux, uy, uz};
    alListenerfv(AL_ORIENTATION, ori);
}

void OpenALAudio::set_listener_velocity(float x, float y, float z)
{
    if (this->is_valid())
        alListener3f(AL_VELOCITY, x, y, z);
}

void OpenALAudio::play_bgm(const std::string &name, bool loop, float volume)
{
    if (!this->is_valid())
        return;
    auto it = this->m_buffers.find(name);
    if (it == this->m_buffers.end()) {
        LOG_CORE_WARN("[OpenALAudio] play_bgm: '{}' not loaded", name);
        return;
    }

    // Cancel any pending stop-after-fade from a previous stop_bgm() call.
    m_bgm_stop_after_fade = false;

    this->m_bgm_individual_vol = std::clamp(volume, 0.f, 1.f);
    alSourceStop(this->m_bgm_source);
    alSourcei(this->m_bgm_source, AL_BUFFER, (ALint)it->second);
    alSourcei(this->m_bgm_source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
    this->apply_bgm_volume();
    alSourcePlay(this->m_bgm_source);
    this->m_bgm_current = name;
    LOG_CORE_INFO("[OpenALAudio] BGM play '{}' (loop={}, vol={:.2f})", name, loop, volume);
}

void OpenALAudio::stop_bgm(float fade_time)
{
    if (!is_valid())
        return;

    if (fade_time <= 0.f) {
        alSourceStop(m_bgm_source);
        return;
    }

    m_bgm_stop_after_fade = true;
    fade_bgm(0.0f, fade_time);
}

void OpenALAudio::pause_bgm()
{
    if (is_valid())
        alSourcePause(this->m_bgm_source);
}

void OpenALAudio::resume_bgm()
{
    if (!this->is_valid())
        return;
    ALint state = AL_STOPPED;
    alGetSourcei(this->m_bgm_source, AL_SOURCE_STATE, &state);
    if (state == AL_PAUSED)
        alSourcePlay(this->m_bgm_source);
}

void OpenALAudio::set_master_volume(float v)
{
    this->m_master_vol = std::clamp(v, 0.f, 1.f);
    if (is_valid())
        alListenerf(AL_GAIN, this->m_master_vol);
}

void OpenALAudio::set_sfx_volume(float v)
{
    this->m_sfx_vol = std::clamp(v, 0.f, 1.f);
    this->reapply_sfx_gains();
    this->reapply_sfx_3d_gains();
}

void OpenALAudio::set_bgm_volume(float v)
{
    this->m_bgm_vol = std::clamp(v, 0.f, 1.f);
    apply_bgm_volume();
}

std::vector<std::string> OpenALAudio::get_loaded_names() const
{
    std::vector<std::string> names;
    names.reserve(this->m_buffers.size());
    for (const auto &[name, _] : this->m_buffers)
        names.push_back(name);
    return names;
}

std::string OpenALAudio::get_bgm_state() const
{
    if (!this->is_valid() || this->m_bgm_current.empty())
        return "none";
    ALint state = AL_STOPPED;
    alGetSourcei(this->m_bgm_source, AL_SOURCE_STATE, &state);
    switch (state) {
    case AL_PLAYING:
        return "playing";
    case AL_PAUSED:
        return "paused";
    default:
        return "stopped";
    }
}

AudioInterface::SoundInfo OpenALAudio::get_sound_info(const std::string &name) const
{
    auto it = m_buffers.find(name);
    if (it == m_buffers.end())
        return {};

    ALint channels = 0, bits = 0, size = 0, freq = 0;
    alGetBufferi(it->second, AL_CHANNELS, &channels);
    alGetBufferi(it->second, AL_BITS, &bits);
    alGetBufferi(it->second, AL_SIZE, &size);
    alGetBufferi(it->second, AL_FREQUENCY, &freq);

    SoundInfo info;
    info.valid = true;
    info.channels = channels;
    info.sample_rate = freq;
    if (freq > 0 && channels > 0 && bits > 0)
        info.duration_sec = (float)size / (float)(channels * freq * (bits >> 3));
    return info;
}

float OpenALAudio::get_bgm_position() const
{
    if (!is_valid() || m_bgm_current.empty())
        return 0.f;
    float pos = 0.f;
    alGetSourcef(m_bgm_source, AL_SEC_OFFSET, &pos);
    return pos;
}

float OpenALAudio::get_bgm_duration() const
{
    if (!is_valid() || m_bgm_current.empty())
        return 0.f;
    return get_sound_info(m_bgm_current).duration_sec;
}

int OpenALAudio::get_active_sfx_count() const
{
    if (!is_valid())
        return 0;
    int n = 0;
    for (int i = 0; i < SFX_POOL; ++i) {
        ALint state = AL_STOPPED;
        alGetSourcei(m_sfx_sources[i], AL_SOURCE_STATE, &state);
        if (state == AL_PLAYING)
            ++n;
    }
    return n;
}

int OpenALAudio::get_active_sfx_3d_count() const
{
    if (!is_valid())
        return 0;
    int n = 0;
    for (int i = 0; i < SFX_3D_POOL; ++i) {
        ALint state = AL_STOPPED;
        alGetSourcei(m_sfx_3d_sources[i], AL_SOURCE_STATE, &state);
        if (state == AL_PLAYING)
            ++n;
    }
    return n;
}

std::vector<AudioInterface::SfxSourceInfo> OpenALAudio::get_sfx_sources_info() const
{
    if (!is_valid())
        return {};

    std::unordered_map<ALuint, std::string> buf_to_name;
    for (const auto &[name, buf] : m_buffers)
        buf_to_name[buf] = name;

    std::vector<SfxSourceInfo> result;

    for (int i = 0; i < SFX_POOL; ++i) {
        ALint state = AL_STOPPED;
        alGetSourcei(m_sfx_sources[i], AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING && state != AL_PAUSED)
            continue;
        SfxSourceInfo info;
        info.index = i;
        info.is_3d = false;
        ALint buf_id = 0;
        alGetSourcei(m_sfx_sources[i], AL_BUFFER, &buf_id);
        auto it = buf_to_name.find((ALuint)buf_id);
        if (it != buf_to_name.end())
            info.sound_name = it->second;
        alGetSourcef(m_sfx_sources[i], AL_GAIN, &info.gain);
        result.push_back(info);
    }

    for (int i = 0; i < SFX_3D_POOL; ++i) {
        ALint state = AL_STOPPED;
        alGetSourcei(m_sfx_3d_sources[i], AL_SOURCE_STATE, &state);
        if (state != AL_PLAYING && state != AL_PAUSED)
            continue;
        SfxSourceInfo info;
        info.index = i;
        info.is_3d = true;
        ALint buf_id = 0;
        alGetSourcei(m_sfx_3d_sources[i], AL_BUFFER, &buf_id);
        auto it = buf_to_name.find((ALuint)buf_id);
        if (it != buf_to_name.end())
            info.sound_name = it->second;
        alGetSourcef(m_sfx_3d_sources[i], AL_GAIN, &info.gain);
        alGetSource3f(m_sfx_3d_sources[i], AL_POSITION, &info.pos[0], &info.pos[1], &info.pos[2]);
        result.push_back(info);
    }

    return result;
}

void OpenALAudio::stop_sfx_source(int index, bool is_3d)
{
    if (!is_valid())
        return;
    if (is_3d) {
        if (index >= 0 && index < SFX_3D_POOL)
            alSourceStop(m_sfx_3d_sources[index]);
    } else {
        if (index >= 0 && index < SFX_POOL)
            alSourceStop(m_sfx_sources[index]);
    }
}

void OpenALAudio::fade_bgm(float target, float duration)
{
    m_bgm_fade.active = true;
    m_bgm_fade.start = m_bgm_fade_gain;
    m_bgm_fade.target = std::clamp(target, 0.f, 1.f);
    m_bgm_fade.duration = duration;
    m_bgm_fade.elapsed = 0.f;
}

void OpenALAudio::set_bgm_fade_gain(float v)
{
    m_bgm_fade_gain = std::clamp(v, 0.f, 1.f);
    apply_bgm_volume();
}

void OpenALAudio::update(float dt)
{
    if (!is_valid())
        return;

    if (m_bgm_fade.active) {
        m_bgm_fade.elapsed += dt;

        float t = m_bgm_fade.duration > 0.f ? m_bgm_fade.elapsed / m_bgm_fade.duration : 1.f;

        if (t >= 1.f) {
            t = 1.f;
            m_bgm_fade.active = false;
        }

        // float v = m_bgm_fade.start + (m_bgm_fade.target - m_bgm_fade.start) * t;

        // float epsilon = 0.0001f;
        // float start = std::max(m_bgm_fade.start, epsilon);
        // float v = start * pow(m_bgm_fade.target / start, t);

        float t2 = t * t * (3.0f - 2.0f * t);
        float v = m_bgm_fade.start + (m_bgm_fade.target - m_bgm_fade.start) * t2;

        m_bgm_fade_gain = v;
        apply_bgm_volume();

        if (!m_bgm_fade.active && m_bgm_stop_after_fade) {
            alSourceStop(m_bgm_source);
            m_bgm_stop_after_fade = false;
            m_bgm_fade_gain = 1.0f; // reset
        }
    }
}

// ============================================================
// Private Methods
// ============================================================

void OpenALAudio::apply_bgm_volume()
{
    if (this->is_valid()) {
        float final = m_bgm_vol * m_bgm_individual_vol * m_bgm_fade_gain;

        alSourcef(this->m_bgm_source, AL_GAIN, std::clamp(final, 0.f, 1.f));
    }
}

int OpenALAudio::next_sfx_source()
{
    for (int i = 0; i < SFX_POOL; ++i) {
        ALint state = AL_STOPPED;
        alGetSourcei(this->m_sfx_sources[i], AL_SOURCE_STATE, &state);
        if (state == AL_STOPPED || state == AL_INITIAL)
            return i;
    }
    int idx = m_sfx_next;
    m_sfx_next = (m_sfx_next + 1) % SFX_POOL;
    return idx;
}

int OpenALAudio::next_sfx_3d_source()
{
    for (int i = 0; i < SFX_3D_POOL; ++i) {
        ALint state = AL_STOPPED;
        alGetSourcei(this->m_sfx_3d_sources[i], AL_SOURCE_STATE, &state);
        if (state == AL_STOPPED || state == AL_INITIAL)
            return i;
    }
    int idx = m_sfx_3d_next;
    m_sfx_3d_next = (m_sfx_3d_next + 1) % SFX_3D_POOL;
    return idx;
}

void OpenALAudio::reapply_sfx_gains()
{
    if (!this->is_valid())
        return;
    for (int i = 0; i < SFX_POOL; ++i)
        alSourcef(this->m_sfx_sources[i], AL_GAIN,
                  std::clamp(this->m_sfx_vol * this->m_sfx_individual_vols[i], 0.f, 1.f));
}

void OpenALAudio::reapply_sfx_3d_gains()
{
    if (!this->is_valid())
        return;
    for (int i = 0; i < SFX_3D_POOL; ++i)
        alSourcef(this->m_sfx_3d_sources[i], AL_GAIN,
                  std::clamp(this->m_sfx_vol * this->m_sfx_3d_individual_vols[i], 0.f, 1.f));
}
