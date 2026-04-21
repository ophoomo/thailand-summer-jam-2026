#ifndef CE0DBB3F_21CF_4A2D_BCB2_3503F16EFFDE
#define CE0DBB3F_21CF_4A2D_BCB2_3503F16EFFDE

#include "renderer/renderer_interface.h"

struct SpriteCommand
{
    float x, y, w, h;
    float u0, v0, u1, v1;
    TextureHandle texture;
    uint32_t color;
    int32_t layer;
    float rotation;  // radians CCW
    float originX, originY;  // absolute offset from (x,y) — pivot point
};

#endif /* CE0DBB3F_21CF_4A2D_BCB2_3503F16EFFDE */
