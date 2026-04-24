#ifndef F9349D06_B140_4792_BB7F_5B0B875FC955
#define F9349D06_B140_4792_BB7F_5B0B875FC955

#include <cmath>
#include <cstdlib>

struct CardOscillator
{
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float phase = 0.0f;
    float timer = 0.0f;

    void randomize()
    {
        phase = (float)(rand() % 360);
        timer = (float)(rand() % 100) / 10.0f;
        frequency = 0.8f + (float)(rand() % 10) / 20.0f;
    }

    float update(double dt)
    {
        timer += (float)dt * frequency;
        return amplitude * std::sin(timer + phase);
    }
};

struct Sway : public CardOscillator
{
    Sway()
    {
        amplitude = 3.0f;
    }
};
struct Hover : public CardOscillator
{
    Hover()
    {
        amplitude = 5.0f;
    }
};
struct Rotate : public CardOscillator
{
    Rotate()
    {
        amplitude = 1.5f;
    }
};

#endif /* F9349D06_B140_4792_BB7F_5B0B875FC955 */
