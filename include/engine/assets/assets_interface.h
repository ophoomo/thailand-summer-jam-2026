#ifndef D5E5F23B_725D_4483_9D31_6A3AF20D3A46
#define D5E5F23B_725D_4483_9D31_6A3AF20D3A46

#include "stb/stb_truetype.h"
#include "string"
#include <nlohmann/json.hpp>

class AssetsInterface
{
  public:
    AssetsInterface() = default;
    ~AssetsInterface() = default;

    virtual unsigned char *loadImage(const char *filename, int &width, int &height,
                                     int &channels) = 0;
    virtual void unLoadImage(unsigned char *image) = 0;
    virtual int loadAudio(const char *filename, int &channels, int &sample_rate, short *&data) = 0;
    virtual nlohmann::json loadJson(const char *filename) = 0;
    virtual std::string loadText(const char *filename) = 0;
};

#endif /* D5E5F23B_725D_4483_9D31_6A3AF20D3A46 */
