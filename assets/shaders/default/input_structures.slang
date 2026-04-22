

module input_structures;

public struct SceneData {
    public float4x4 view;
    public float4x4 proj;
    public float4x4 viewproj;
    public float4   ambientColor;
    public float4   sunlightDirection;
    public float4   sunlightColor;
};

public struct GLTFMaterialData {
    public float4 colorFactors;
    public float4 metal_rough_factors;
};

[vk::binding(0, 0)]
public ConstantBuffer<SceneData> sceneData;

[vk::binding(0, 1)]
public ConstantBuffer<GLTFMaterialData> materialData;

[vk::binding(1, 1)]
public Texture2D colorTex;

[vk::binding(2, 1)]
public Texture2D metalRoughTex;

