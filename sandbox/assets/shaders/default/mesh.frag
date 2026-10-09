
import input_structures;

struct FragmentInput {
    float3 normal : NORMAL;
    float3 color  : COLOR;
    float2 uv     : TEXCOORD0;
};

[shader("fragment")]
float4 main(FragmentInput input) : SV_Target
{
    float lightValue = max(dot(input.normal, sceneData.sunlightDirection.xyz), 0.1f);

    float3 color   = input.color * colorTex.Sample(input.uv).xyz;
    float3 ambient = color * sceneData.ambientColor.xyz;

    return float4(color * lightValue * sceneData.sunlightColor.w + ambient, 1.0f);
}
