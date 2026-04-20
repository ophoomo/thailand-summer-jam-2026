
#include "assets/raw/raw_assets.h"
#include "assets/stb_vorbis_wrapper.h"
#include "stb/stb_image.h"
#include "utils/logger.h"
#include "utils/resource_path.h"
#include <fstream>
#include <vector>

// ============================================================
// Construction / destruction
// ============================================================

RawAssets::RawAssets() {}

RawAssets::~RawAssets() {}

// ============================================================
// Public Methods
// ============================================================

unsigned char *RawAssets::loadImage(const char *filename, int &width, int &height, int &channels)
{
    unsigned char *img = stbi_load(ResourcePath::resolve(filename).c_str(), &width, &height,
                                   &channels, STBI_rgb_alpha);
    if (img == nullptr) {
        LOG_CORE_ERROR("[Assets] Error: Could not load image {}", filename);
        return nullptr;
    }

    LOG_CORE_TRACE("[Assets] Load Image {} Success", filename);
    return img;
}

void RawAssets::unLoadImage(unsigned char *image)
{
    stbi_image_free(image);
}

int RawAssets::loadAudio(const char *filename, int &channels, int &sample_rate, short *&data)
{
    int samples = stb_vorbis_decode_filename(ResourcePath::resolve(filename).c_str(), &channels,
                                             &sample_rate, &data);
    if (samples <= 0) {
        LOG_CORE_ERROR("[Assets] Error: Could not load audio {}", filename);
        return samples;
    }
    LOG_CORE_TRACE("[Assets] Load Audio {} Success", filename);
    return samples;
}

std::string RawAssets::loadText(const char *filename)
{
    auto text = this->load_file(filename);
    std::string s(text.begin(), text.end());
    return s;
}

nlohmann::json RawAssets::loadJson(const char *filename)
{
    std::ifstream file(ResourcePath::resolve(filename));
    if (!file.is_open()) {
        auto error = std::format("[Assets] Failed to open file: {}", filename);
        LOG_CORE_ERROR(error);
        throw std::runtime_error(error);
    }

    try {
        nlohmann::json j;
        file >> j;
        return j;
    } catch (const nlohmann::json::parse_error &e) {
        LOG_CORE_ERROR("[Assets] JSON parse error in file {}: {}", filename, e.what());
        throw;
    }
}

// ============================================================
// Private Methods
// ============================================================

std::vector<unsigned char> RawAssets::load_file(const char *filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error(std::string("Failed to open file: ") + filename);
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<unsigned char> buffer(size);
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size)) {
        throw std::runtime_error(std::string("Failed to read file: ") + filename);
    }
    return buffer;
}
