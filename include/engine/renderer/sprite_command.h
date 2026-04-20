#ifndef CE0DBB3F_21CF_4A2D_BCB2_3503F16EFFDE
#define CE0DBB3F_21CF_4A2D_BCB2_3503F16EFFDE

#include "renderer/renderer_interface.h"

struct SpriteCommand
{
    float x, y, w, h;
    TextureHandle texture;
    uint32_t color;
    int32_t layer;
};

#endif /* CE0DBB3F_21CF_4A2D_BCB2_3503F16EFFDE */
