#include "Sprite2D.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLighting; // スプライトでは使わないが、パディング合わせで残しておくのが安全
    float32_t3 padding;
    float32_t4x4 uvTransform;
};

// Sprite.cppの実装に合わせて register(b0) に設定
// (RootParam 0 に割り当てられていると仮定)
ConstantBuffer<Material> gMaterial : register(b0);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_Target0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    
    // UV変換 (必要なら)
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    
    // テクスチャサンプリング
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    // マテリアル色とテクスチャ色を合成
    output.color = gMaterial.color * textureColor;
    
    // アルファテスト: 完全に透明なピクセルは描画しない
    if (output.color.a == 0.0f)
    {
        discard;
    }
    
    return output;
}