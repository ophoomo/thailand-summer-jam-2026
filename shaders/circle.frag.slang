
struct PSInput
{
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float texSlot : TEXCOORD1;
};

[shader("fragment")]
float4 main(PSInput input) : SV_Target
{
    float2 d = input.uv - float2(0.5, 0.5);
    float dist = length(d);
    float aa = fwidth(dist);
    float alpha = 1.0 - smoothstep(0.5 - aa, 0.5 + aa, dist);
    if (alpha <= 0.0)
        discard;
    return float4(input.color.rgb, input.color.a * alpha);
}
