#ifndef B6CDEAA5_4BDD_4436_B22D_9D253CAD1813
#define B6CDEAA5_4BDD_4436_B22D_9D253CAD1813
#ifndef FD62981E_31F0_4A25_815D_DA639B0941B2
#define FD62981E_31F0_4A25_815D_DA639B0941B2

#include "renderer/text_effect.h"
#include <cstdint>
#include <string>

struct TextCommand
{
    std::string text;
    float x, y, size;
    uint32_t color;
    int32_t layer;
    TextEffect effect;
    float rotation;  // radians CCW
    float pivotX, pivotY;  // absolute world pivot point
};

#endif /* FD62981E_31F0_4A25_815D_DA639B0941B2 */

#endif /* B6CDEAA5_4BDD_4436_B22D_9D253CAD1813 */
