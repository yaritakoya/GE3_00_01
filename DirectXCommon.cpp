#include "DirectXCommon.h"
#include <cassert>
#include <dxcapi.h>
#include <format>
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "WinApp.h"
#include "Logger.h"
#include "StringUtility.h"
#include <string>
#include <sstream>
#include <Windows.h>
#include <dxgidebug.h> 
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

using namespace Microsoft::WRL;
using namespace StringUtility;

void DirectXCommon::Initialize(WinApp* winApp) {

    assert(winApp);
    winApp_ = winApp;

    InitializeDevice();
    InitializeCommand();
    InitializeSwapChain(winApp);
    InitializeDescriptorHeaps();
    InitializeRenderTargetView();
    InitializeDepthBuffer();
    InitializeDepthStencilView();
    InitializeViewport();
    InitializeScissorRect();
    InitializeDXCCompiler();
    InitializeImGui();
    InitializeFence();
}

// =====================================
//  描画前処理 / 描画後処理
// =====================================

void DirectXCommon::PreDraw() {

    // バックバッファの番号
    UINT bbIndex = swapChain_->GetCurrentBackBufferIndex();

    // PRESENT → RENDER_TARGET
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = backBuffers_[bbIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList_->ResourceBarrier(1, &barrier);

    // RTV
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
        rtvHeap_->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += static_cast<SIZE_T>(rtvDescriptorSize_) * bbIndex;

    // DSV はヒープ先頭
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle =
        dsvHeap_->GetCPUDescriptorHandleForHeapStart();

    // 描画先設定
    commandList_->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // 画面クリア
    float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
    commandList_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    commandList_->ClearDepthStencilView(
        dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // ビューポート / シザー
    commandList_->RSSetViewports(1, &viewport_);
    commandList_->RSSetScissorRects(1, &scissorRect_);
}

void DirectXCommon::PostDraw() {

    UINT bbIndex = swapChain_->GetCurrentBackBufferIndex();

    // RENDER_TARGET → PRESENT
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = backBuffers_[bbIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList_->ResourceBarrier(1, &barrier);

    // コマンドリストを閉じる
    HRESULT hr = commandList_->Close();
    assert(SUCCEEDED(hr));

    // GPU に送る
    ID3D12CommandList* lists[] = { commandList_.Get() };
    commandQueue_->ExecuteCommandLists(1, lists);

    // フリップ
    hr = swapChain_->Present(1, 0);
    assert(SUCCEEDED(hr));

    // フェンス
    fenceVal_++;
    commandQueue_->Signal(fence_.Get(), fenceVal_);

    if (fence_->GetCompletedValue() < fenceVal_) {
        fence_->SetEventOnCompletion(fenceVal_, fenceEvent_);
        WaitForSingleObject(fenceEvent_, INFINITE);
    }

    // 次フレーム用にリセット
    hr = commandAllocator_->Reset();
    assert(SUCCEEDED(hr));
    hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
    assert(SUCCEEDED(hr));
}

// =====================================
//  SRV ハンドル
// =====================================

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVCPUDescriptorHandle(uint32_t index) const {
    return GetCPUDescriptorHandle(srvDescriptorHeap_, srvDescriptorSize_, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVGPUDescriptorHandle(uint32_t index) const {
    return GetGPUDescriptorHandle(srvDescriptorHeap_, srvDescriptorSize_, index);
}

// =====================================
//  デバイス / コマンド / スワップチェーン
// =====================================

void DirectXCommon::InitializeDevice() {

    HRESULT hr;

#if _DEBUG
    ComPtr<ID3D12Debug1> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
        debugController->SetEnableGPUBasedValidation(TRUE);
    }
#endif

    hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgiFactory_));
    assert(SUCCEEDED(hr));

    hr = dxgiFactory_->EnumAdapterByGpuPreference(
        0,
        DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
        IID_PPV_ARGS(&adapter_));
    assert(SUCCEEDED(hr));

    hr = D3D12CreateDevice(
        adapter_.Get(),
        D3D_FEATURE_LEVEL_12_0,
        IID_PPV_ARGS(&device_));
    assert(SUCCEEDED(hr));
}

void DirectXCommon::InitializeCommand() {

    HRESULT hr;

    // コマンドキュー
    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    hr = device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue_));
    assert(SUCCEEDED(hr));

    // アロケータ
    hr = device_->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&commandAllocator_));
    assert(SUCCEEDED(hr));

    // コマンドリスト
    hr = device_->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        commandAllocator_.Get(),
        nullptr,
        IID_PPV_ARGS(&commandList_));
    assert(SUCCEEDED(hr));
}

void DirectXCommon::InitializeSwapChain(WinApp* winApp) {

    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Width = WinApp::kClientWidth;
    desc.Height = WinApp::kClientHeight;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    ComPtr<IDXGISwapChain1> swapChain1;
    HRESULT hr = dxgiFactory_->CreateSwapChainForHwnd(
        commandQueue_.Get(),
        winApp->GetHwnd(),
        &desc,
        nullptr,
        nullptr,
        &swapChain1);
    assert(SUCCEEDED(hr));

    hr = swapChain1.As(&swapChain_);
    assert(SUCCEEDED(hr));
}

// =====================================
//  デスクリプタヒープ / RTV / DSV
// =====================================

void DirectXCommon::InitializeDescriptorHeaps() {
    HRESULT hr;

    // RTV
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = 2;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        hr = device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvHeap_));
        assert(SUCCEEDED(hr));

        rtvDescriptorSize_ =
            device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // DSV
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        desc.NumDescriptors = 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        hr = device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&dsvHeap_));
        assert(SUCCEEDED(hr));

        dsvDescriptorSize_ =
            device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    }

    // SRV
    {
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 128;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        hr = device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srvDescriptorHeap_));
        assert(SUCCEEDED(hr));

        srvDescriptorSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    }
}

void DirectXCommon::InitializeRenderTargetView() {

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    D3D12_CPU_DESCRIPTOR_HANDLE handle =
        rtvHeap_->GetCPUDescriptorHandleForHeapStart();

    for (UINT i = 0; i < 2; ++i) {
        HRESULT hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(&backBuffers_[i]));
        assert(SUCCEEDED(hr));

        device_->CreateRenderTargetView(backBuffers_[i].Get(), &rtvDesc, handle);

        handle.ptr += static_cast<SIZE_T>(rtvDescriptorSize_);
    }
}

void DirectXCommon::InitializeDepthBuffer() {

    D3D12_RESOURCE_DESC desc{};
    desc.Width = WinApp::kClientWidth;
    desc.Height = WinApp::kClientHeight;
    desc.MipLevels = 1;
    desc.DepthOrArraySize = 1;
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.SampleDesc.Count = 1;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_CLEAR_VALUE clearValue{};
    clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    HRESULT hr = device_->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue,
        IID_PPV_ARGS(&depthBuffer_));
    assert(SUCCEEDED(hr));
}

void DirectXCommon::InitializeDepthStencilView() {

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

    D3D12_CPU_DESCRIPTOR_HANDLE handle =
        dsvHeap_->GetCPUDescriptorHandleForHeapStart();

    device_->CreateDepthStencilView(depthBuffer_.Get(), &dsvDesc, handle);
}

void DirectXCommon::InitializeViewport() {
    viewport_.Width = static_cast<float>(WinApp::kClientWidth);
    viewport_.Height = static_cast<float>(WinApp::kClientHeight);
    viewport_.TopLeftX = 0.0f;
    viewport_.TopLeftY = 0.0f;
    viewport_.MinDepth = 0.0f;
    viewport_.MaxDepth = 1.0f;
}

void DirectXCommon::InitializeScissorRect() {
    scissorRect_.left = 0;
    scissorRect_.top = 0;
    scissorRect_.right = WinApp::kClientWidth;
    scissorRect_.bottom = WinApp::kClientHeight;
}

void DirectXCommon::InitializeDXCCompiler() {
    HRESULT hr = DxcCreateInstance(
        CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
    assert(SUCCEEDED(hr));

    hr = DxcCreateInstance(
        CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
    assert(SUCCEEDED(hr));

    hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
    assert(SUCCEEDED(hr));
}

void DirectXCommon::InitializeImGui() {
   // バージョンチェック
   IMGUI_CHECKVERSION();

   // コンテキストの生成
   ImGui::CreateContext();

   // スタイルの設定
   ImGui::StyleColorsDark();

   // Win32用の初期化
   ImGui_ImplWin32_Init(winApp_->GetHwnd());

   // DirectX12用の初期化
   ImGui_ImplDX12_Init(
       device_.Get(), //ComPtr から生のポインタを取得
       2, // バッファ数
       DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, // フォーマット
       srvDescriptorHeap_.Get(), // SRVヒープ
       srvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart(), // CPUハンドル
       srvDescriptorHeap_->GetGPUDescriptorHandleForHeapStart()  // GPUハンドル
   );
}

void DirectXCommon::InitializeFence() {
    HRESULT hr = device_->CreateFence(
        fenceVal_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
    assert(SUCCEEDED(hr));

    fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    assert(fenceEvent_ != nullptr);
}


D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(
    const ComPtr<ID3D12DescriptorHeap>& heap,
    UINT size,
    uint32_t index) {

    D3D12_CPU_DESCRIPTOR_HANDLE handle =
        heap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(size) * index;
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(
    const ComPtr<ID3D12DescriptorHeap>& heap,
    UINT size,
    uint32_t index) {

    D3D12_GPU_DESCRIPTOR_HANDLE handle =
        heap->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<UINT64>(size) * index;
    return handle;
}