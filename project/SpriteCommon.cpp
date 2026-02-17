#include "SpriteCommon.h"



void SpriteCommon::Initialize(DirectXCommon* dxCommon) {

	// スプライト共通部の初期化処理をここに記述
	dxCommon_ = dxCommon;

    // ① ルートシグネチャ作成
   /* CreateRootSignature();*/

    // ② グラフィックスパイプライン作成
    CreateGraphicsPipelineState();

}

void SpriteCommon::CreateRootSignature() {

    ID3D12Device* device = dxCommon_->GetDevice();

    // ▼ main.cpp から移植
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
    rootSigDesc.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_STATIC_SAMPLER_DESC staticSamplers[1]{};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    rootSigDesc.pStaticSamplers = staticSamplers;
    rootSigDesc.NumStaticSamplers = _countof(staticSamplers);



    D3D12_DESCRIPTOR_RANGE descriptorRange{};
    descriptorRange.BaseShaderRegister = 0;
    descriptorRange.NumDescriptors = 1;
    descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange.OffsetInDescriptorsFromTableStart =
        D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;



    D3D12_ROOT_PARAMETER rootParameters[4]{};

    // [0] Material (CBV, Pixel)
    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    // [1] Transform / WVP (CBV, Vertex)
    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].Descriptor.ShaderRegister = 0;

    rootParameters[2].ParameterType =
        D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility =
        D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

    // [3] Directional Light (CBV, Pixel)
    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[3].Descriptor.ShaderRegister = 1;

    rootSigDesc.pParameters = rootParameters;
    rootSigDesc.NumParameters = _countof(rootParameters);

    // static sampler や root parameter も main.cpp のをそのままコピペ

    ID3DBlob* signatureBlob = nullptr;
    ID3DBlob* errorBlob = nullptr;

    HRESULT hr = D3D12SerializeRootSignature(
        &rootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signatureBlob,
        &errorBlob
    );
   
    hr = device->CreateRootSignature(
        0,
        signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature_)   // ★ SpriteCommon のメンバに格納
    );
    assert(SUCCEEDED(hr));

    // 解放
    signatureBlob->Release();
    if (errorBlob) { errorBlob->Release(); }
}


void SpriteCommon::CreateGraphicsPipelineState() {

    // ルートシグネチャ作成
    CreateRootSignature();

    ID3D12Device* device = dxCommon_->GetDevice();

    // ----- InputLayout -----
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};

    // POSITION
    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    // TEXCOORD
    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	// NORMAL
    inputElementDescs[2].SemanticName = "NORMAL";
    inputElementDescs[2].SemanticIndex = 0;
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;



    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);


    // ----- Blend（アルファブレンド） -----
    D3D12_BLEND_DESC blendDesc{};
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;


    // ----- Rasterizer -----
    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;


    // ----- DepthStencil（Sprite は奥行不要） -----
    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = FALSE;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;


    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob =
        dxCommon_->CompileShader(L"Resources/shader/Object3D.VS.hlsl", L"vs_6_0");

    Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob =
        dxCommon_->CompileShader(L"Resources/shader/Object3D.PS.hlsl", L"ps_6_0");



    // ----- PSO -----
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.pRootSignature = rootSignature_.Get();
    psoDesc.InputLayout = inputLayoutDesc;
    psoDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
    psoDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
    psoDesc.BlendState = blendDesc;
    psoDesc.RasterizerState = rasterizerDesc;
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

    HRESULT hr = device->CreateGraphicsPipelineState(
        &psoDesc,
        IID_PPV_ARGS(&pipelineState_));
    assert(SUCCEEDED(hr));
}


void SpriteCommon::PreDraw() {

    // コマンドリスト取得
    ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

    // ルートシグネチャをセット
    commandList->SetGraphicsRootSignature(rootSignature_.Get());

    // パイプラインステートをセット
    commandList->SetPipelineState(pipelineState_.Get());

    // プリミティブトポロジセット（スプライトは三角形）
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
