#pragma once

#include "WinApp.h"

#include <array>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <wrl.h>
#include <string>
#include <chrono>

#include "externals/DirectXTex/DirectXTex.h"

class DirectXCommon {
public:
    // 初期化 / 描画前後
    void Initialize(WinApp* winApp);
    void PreDraw();
    void PostDraw();

    // SRV ハンドル取得
    D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index) const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index) const;

    // SRVヒープの取得
    ID3D12DescriptorHeap* GetSrvHeap() const { return srvDescriptorHeap_.Get(); }

    ID3D12Device* GetDevice() const { return device_.Get(); }
    ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }

    // ★★★ 復活：テクスチャ読み込み関数 ★★★
    DirectX::ScratchImage LoadTexture(const std::string& filePath);

    // ===== ヘルパー関数 =====
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes);
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metadata);
    Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);
    Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const std::wstring& profile);

private:
    void InitializeFixFPS();
    void InitializeDevice();
    void InitializeCommand();
    void InitializeSwapChain(WinApp* winApp);
    void InitializeDescriptorHeaps();
    void InitializeRenderTargetView();
    void InitializeDepthBuffer();
    void InitializeDepthStencilView();
    void InitializeViewport();
    void InitializeScissorRect();
    void InitializeDXCCompiler();
    void InitializeImGui();
    void InitializeFence();

    WinApp* winApp_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Device> device_;
    Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_;

    // コマンドキュー（これがないとGPUが動きません）
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_;

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_;

    // スワップチェーン & バックバッファ
    Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_;
    std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> backBuffers_;

    // RTV / DSV
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap_;
    UINT rtvDescriptorSize_ = 0;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvHeap_;
    Microsoft::WRL::ComPtr<ID3D12Resource> depthBuffer_;
    UINT dsvDescriptorSize_ = 0;

    // SRV 用ヒープ
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
    UINT srvDescriptorSize_ = 0;

    // フェンス
    Microsoft::WRL::ComPtr<ID3D12Fence> fence_;
    UINT64 fenceVal_ = 0;
    HANDLE fenceEvent_ = nullptr;

    // ビューポート / シザー
    D3D12_VIEWPORT viewport_{};
    D3D12_RECT     scissorRect_{};

    // DXC
    Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
    Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
};