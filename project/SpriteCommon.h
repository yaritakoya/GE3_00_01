#pragma once
#include "DirectXCommon.h"
#include "Logger.h"
#include <wrl.h>
#include <d3d12.h>

class SpriteCommon {
public:
    void Initialize(DirectXCommon* dxCommon);

    ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
    ID3D12PipelineState* GetPipelineState() const { return pipelineState_.Get(); }

    void PreDraw();



public:
	DirectXCommon* GetDxCommon() const { return dxCommon_; }

private:
    void CreateRootSignature();
    void CreateGraphicsPipelineState();

    DirectXCommon* dxCommon_ = nullptr;

    

  

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
};
