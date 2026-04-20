[[vk::binding(0, 0)]]
Sampler2D u_atlas;

[[vk::push_constant]]
cbuffer Push
{
    float2 screenSize;   // offset  0 — vertex shader
    float2 unitRange;    // offset  8 — pxRange / atlasSize
    float4 effectColor;  // offset 16 — outline/glow color (rgba 0-1)
    float effectParam0;  // offset 32 — outline: width; glow: range
    float effectParam1;  // offset 36 — glow: strength
    int effectType;      // offset 40 — 0=none 1=outline 2=glow
};

struct PSInput
{
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float texSlot : TEXCOORD1;
};

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

[shader("fragment")]
float4 main(PSInput input) : SV_Target
{
    float3 msd = u_atlas.Sample(input.uv).rgb;
    float sd = median(msd.r, msd.g, msd.b);

    float2 screenTexSize = float2(1.0) / fwidth(input.uv);
    float screenPxRange = max(0.5 * dot(unitRange, screenTexSize), 1.0);

    float fillAlpha = clamp(screenPxRange * (sd - 0.5) + 0.5, 0.0, 1.0);

    if (effectType == 1)
    {
        // Outline: expand the glyph edge outward by effectParam0 SDF units.
        // outlineAlpha covers fill + outline region; fillAlpha covers only fill.
        float outlineAlpha = clamp(screenPxRange * (sd - (0.5 - effectParam0)) + 0.5, 0.0, 1.0);
        float3 rgb = lerp(effectColor.rgb, input.color.rgb, fillAlpha);
        float alpha = input.color.a * outlineAlpha;
        clip(alpha - 0.001);
        return float4(rgb, alpha);
    }

    if (effectType == 2)
    {
        // Glow: soft halo that fades from the glyph edge outward.
        // effectParam0 = glow range in SDF units, effectParam1 = strength.
        float glowRange = max(effectParam0, 0.001);
        float glowSd = clamp((sd - (0.5 - glowRange)) / glowRange, 0.0, 1.0);
        float glowAlpha = glowSd * glowSd * effectParam1 * effectColor.a;
        float alpha = max(glowAlpha, fillAlpha * input.color.a);
        float3 rgb = lerp(effectColor.rgb, input.color.rgb, fillAlpha);
        clip(alpha - 0.001);
        return float4(rgb, alpha);
    }

    // No effect
    clip(fillAlpha - 0.001);
    return float4(input.color.rgb, input.color.a * fillAlpha);
}
