#ifndef B3139E60_96EA_4D97_BA6A_9CEB1C0EA883
#define B3139E60_96EA_4D97_BA6A_9CEB1C0EA883

#include "assets/assets_interface.h"

class OxenAssests : public AssetsInterface
{
  public:
    OxenAssests();
    ~OxenAssests();

    virtual unsigned char *loadImage(const char *filename, int &width, int &height,
                                     int &channels) override;
    virtual void unLoadImage(unsigned char *image) override;
};

#endif /* B3139E60_96EA_4D97_BA6A_9CEB1C0EA883 */
