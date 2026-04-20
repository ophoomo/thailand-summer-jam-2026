#ifndef C70BC1BE_CE37_4219_BFCF_A90CCDF6E11C
#define C70BC1BE_CE37_4219_BFCF_A90CCDF6E11C

#include "assets/assets_interface.h"
#include <vector>

class RawAssets : public AssetsInterface
{
  public:
    RawAssets();
    ~RawAssets();

    unsigned char *loadImage(const char *filename, int &width, int &height, int &channels) override;
    void unLoadImage(unsigned char *image) override;
    int loadAudio(const char *filename, int &channels, int &sample_rate, short *&data) override;
    nlohmann::json loadJson(const char *filename) override;
    std::string loadText(const char *filename) override;

  private:
    std::vector<unsigned char> load_file(const char *filename);
};

#endif /* C70BC1BE_CE37_4219_BFCF_A90CCDF6E11C */
