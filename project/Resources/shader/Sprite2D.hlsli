struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION; // システム用頂点座標
    float32_t2 texcoord : TEXCOORD0; // テクスチャ座標
};