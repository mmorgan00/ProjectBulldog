
import input_structures;

struct Vertex {
    float3 position;
    float  uv_x;
    float3 normal;
    float  uv_y;
    float4 color;
};

struct PushConstants {
    float4x4 render_matrix;
    Vertex*  vertexBuffer;
};

[vk::push_constant]
uniform PushConstants pushConstants;

struct VertexOutput {
    float4 position : SV_Position;
    float3 normal   : NORMAL;
    float3 color    : COLOR;
    float2 uv       : TEXCOORD0;
};

[shader("vertex")]
VertexOutput main(uint vertexIndex : SV_VulkanVertexID)
{
    Vertex v = pushConstants.vertexBuffer[vertexIndex];

    VertexOutput output;
    output.position = mul(sceneData.viewproj, mul(pushConstants.render_matrix, float4(v.position, 1.0f)));
    output.normal   = mul(pushConstants.render_matrix, float4(v.normal, 0.f)).xyz;
    output.color    = v.color.xyz * materialData.colorFactors.xyz;
    output.uv.x     = v.uv_x;
    output.uv.y     = v.uv_y;
    return output;
}

