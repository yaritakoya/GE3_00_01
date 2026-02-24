#pragma once
#include "DirectXCommon.h"
#include <string>
#include <vector>
#include <map>
#include <wrl.h>
#include <d3d12.h>

class SpriteCommon {
public:
    void Initialize(DirectXCommon* dxCommon);

    // 描画前の準備
    void PreDraw();

    // ★ テクスチャ読み込み（戻り値はテクスチャハンドル＝配列のインデックス）
    uint32_t LoadTexture(const std::string& filePath);

    // ★ ゲッター
    DirectXCommon* GetDxCommon() const { return dxCommon_; }
    ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
    ID3D12PipelineState* GetPipelineState() const { return pipelineState_.Get(); }

    // 指定番号のSRVハンドル(GPU)を取得
    D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureIndex);

    // 指定番号のテクスチャ情報（幅・高さなど）を取得
    D3D12_RESOURCE_DESC GetTextureResourceDesc(uint32_t textureIndex);


private:
    void CreateRootSignature();
    void CreateGraphicsPipelineState();

    DirectXCommon* dxCommon_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

    // ★ テクスチャ管理用のコンテナ
    // テクスチャリソースの配列
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResources_;
    // テクスチャアップロード用の中間リソース（転送が終わるまで保持しておく必要がある）
    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResources_;

    // パスとインデックスの対応マップ（同じ画像を何度も読み込まないように）
    std::map<std::string, uint32_t> textureMap_;

    // 次に使うSRVインデックス（0番目はImGuiなどが使うかもしれないので1から始める等の調整可能。今回は0から）
    uint32_t srvIndex_ = 0;
};