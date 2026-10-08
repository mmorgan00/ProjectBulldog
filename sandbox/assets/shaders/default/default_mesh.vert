
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
    float3 color    : COLOR;
    float2 uv       : TEXCOORD0;
};

[shader("vertex")]
VertexOutput main(uint vertexIndex : SV_VulkanVertexID)
{
    Vertex v = pushConstants.vertexBuffer[vertexIndex];

    VertexOutput output;
    output.position = mul(pushConstants.render_matrix, float4(v.position, 1.0f));
    output.color    = v.color.xyz;
    output.uv.x     = v.uv_x;
    output.uv.y     = v.uv_y;

    return output;
}

