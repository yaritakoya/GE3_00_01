#include "Object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
};

struct DirecctionalLight
{
    float32_t4 color; //!<ライトの色
    float32_t3 direction; //!< ライトの向き
    float intensity; //!<輝度
};

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);
ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirecctionalLight> gDirectionalLight : register(b1);

struct PixelShaderOutput
{
    float32_t4 color : SV_Target0;
};




PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gMaterial.color;
    

    
     // UV変換
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    //textureのaの値が０．５以下の時にPixelを棄却 
    if (textureColor.a <= 0.5)
    {
        discard;
    }
        
   
    
    if (gMaterial.enableLighting != 0)//Lightingする場合
    {
        
        //textureのaの値が０の時にPixelを棄却
        if (textureColor.a == 0.0)
        {
            discard;
        }
        
        //output.colorのaの値が０の時にPixelを棄却
        if (output.color.a == 0.0)
        {
            discard;
        }
        float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
        output.color.a = gMaterial.color.a * textureColor.a;
        
         
        
    
       
    }
    else
    {
    
        output.color = gMaterial.color * textureColor;
        
                
       
        
    }
    
    

    
    
    return output;
    
}

