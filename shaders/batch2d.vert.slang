
[[vk::push_constant]]
cbuffer Push
{
    float2 screenSize;
};

struct VSInput
{
    [[vk::location(0)]]
    float2 position : POSITION;
    [[vk::location(1)]]
    float2 uv : TEXCOORD0;
    [[vk::location(2)]]
    float4 color : COLOR0; // VK_FORMAT_R8G8B8A8_UNORM → [0,1]
    [[vk::location(3)]]
    float texSlot : TEXCOORD1;
};

struct VSOutput
{
    float4 svPos : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
    float texSlot : TEXCOORD1;
};

[shader("vertex")]
VSOutput main(VSInput input)
{
    VSOutput output;
    float2 ndc = (input.position / screenSize) * 2.0 - 1.0;
    output.svPos = float4(ndc, 0.0, 1.0);
    output.uv = input.uv;
    output.color = input.color;
    output.texSlot = input.texSlot;
    return output;
}
