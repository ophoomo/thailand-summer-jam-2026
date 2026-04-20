[[vk::binding(0, 0)]]
Sampler2D u_tex;

struct PSInput
{
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float texSlot : TEXCOORD1;
};

[shader("fragment")]
float4 main(PSInput input) : SV_Target
{
    float4 c = u_tex.Sample(input.uv) * input.color;
    clip(c.a - 0.001);
    return c;
}
