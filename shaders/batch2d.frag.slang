
struct PSInput
{
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float texSlot : TEXCOORD1;
};

[shader("fragment")]
float4 main(PSInput input) : SV_Target
{
    return input.color;
}
