#include "DirectXCommon.h"

#include <cassert>
#include <vector>
#include <thread>
#include <format>
#include <string>

#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/DirectXTex/d3dx12.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxcompiler.lib")

using namespace Microsoft::WRL;

// =========================================================================
// 初期化関連
// =========================================================================
void DirectXCommon::Initialize(WinApp* winApp) {
    assert(winApp);
    winApp_ = winApp;

    // FPS 固定初期化
    InitializeFixFPS();

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

void DirectXCommon::InitializeDevice() {
#ifdef _DEBUG
    ComPtr<ID3D12Debug1> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
        debugController->SetEnableGPUBasedValidation(TRUE);
    }
#endif

    HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
    assert(SUCCEEDED(hr));

    ComPtr<IDXGIAdapter4> useAdapter;
    for (UINT i = 0; dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC3 adapterDesc{};
        hr = useAdapter->GetDesc3(&adapterDesc);
        assert(SUCCEEDED(hr));
        if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
            adapter_ = useAdapter;
            break;
        }
    }
    assert(adapter_ != nullptr);

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0 };
    for (size_t i = 0; i < _countof(featureLevels); ++i) {
        hr = D3D12CreateDevice(adapter_.Get(), featureLevels[i], IID_PPV_ARGS(&device_));
        if (SUCCEEDED(hr)) { break; }
    }
    assert(device_ != nullptr);

#ifdef _DEBUG
    ComPtr<ID3D12InfoQueue> infoQueue;
    if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
    }
#endif
}

void DirectXCommon::InitializeCommand() {
    HRESULT hr;
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
    hr = device_->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue_));
    assert(SUCCEEDED(hr));

    hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator_));
    assert(SUCCEEDED(hr));

    hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
    assert(SUCCEEDED(hr));
}

void DirectXCommon::InitializeSwapChain(WinApp* winApp) {
    HRESULT hr;
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
    swapChainDesc.Width = WinApp::kClientWidth;
    swapChainDesc.Height = WinApp::kClientHeight;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    ComPtr<IDXGISwapChain1> swapChain1;
    hr = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_.Get(), winApp->GetHwnd(), &swapChainDesc, nullptr, nullptr, &swapChain1);
    assert(SUCCEEDED(hr));
    hr = swapChain1.As(&swapChain_);
    assert(SUCCEEDED(hr));
}

void DirectXCommon::InitializeDescriptorHeaps() {
    HRESULT hr;
    // RTV
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = 2;
    hr = device_->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap_));
    assert(SUCCEEDED(hr));
    rtvDescriptorSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    // DSV
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.NumDescriptors = 1;
    hr = device_->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap_));
    assert(SUCCEEDED(hr));
    dsvDescriptorSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    // SRV
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.NumDescriptors = 128;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    hr = device_->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap_));
    assert(SUCCEEDED(hr));
    srvDescriptorSize_ = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void DirectXCommon::InitializeRenderTargetView() {
    HRESULT hr;
    hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&backBuffers_[0]));
    assert(SUCCEEDED(hr));
    hr = swapChain_->GetBuffer(1, IID_PPV_ARGS(&backBuffers_[1]));
    assert(SUCCEEDED(hr));

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    for (UINT i = 0; i < 2; ++i) {
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
        rtvHandle.ptr += static_cast<SIZE_T>(rtvDescriptorSize_) * i;
        device_->CreateRenderTargetView(backBuffers_[i].Get(), &rtvDesc, rtvHandle);
    }
}

void DirectXCommon::InitializeDepthBuffer() {
    D3D12_RESOURCE_DESC depthResDesc{};
    depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthResDesc.Width = WinApp::kClientWidth;
    depthResDesc.Height = WinApp::kClientHeight;
    depthResDesc.DepthOrArraySize = 1;
    depthResDesc.MipLevels = 1;
    depthResDesc.SampleDesc.Count = 1;
    depthResDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_HEAP_PROPERTIES heapProps{};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_CLEAR_VALUE depthClearValue{};
    depthClearValue.DepthStencil.Depth = 1.0f;
    depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    HRESULT hr = device_->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE,
        &depthResDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &depthClearValue, IID_PPV_ARGS(&depthBuffer_));
    assert(SUCCEEDED(hr));
}

void DirectXCommon::InitializeDepthStencilView() {
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    device_->CreateDepthStencilView(depthBuffer_.Get(), &dsvDesc, dsvHeap_->GetCPUDescriptorHandleForHeapStart());
}

void DirectXCommon::InitializeViewport() {
    viewport_.Width = static_cast<FLOAT>(WinApp::kClientWidth);
    viewport_.Height = static_cast<FLOAT>(WinApp::kClientHeight);
    viewport_.TopLeftX = 0;
    viewport_.TopLeftY = 0;
    viewport_.MinDepth = 0.0f;
    viewport_.MaxDepth = 1.0f;
}

void DirectXCommon::InitializeScissorRect() {
    scissorRect_.left = 0;
    scissorRect_.right = WinApp::kClientWidth;
    scissorRect_.top = 0;
    scissorRect_.bottom = WinApp::kClientHeight;
}

void DirectXCommon::InitializeDXCCompiler() {
    // コンパイラ初期化用の関数（今回の中身はmainで実装されているため空）
}

void DirectXCommon::InitializeImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(winApp_->GetHwnd());
    ImGui_ImplDX12_Init(
        device_.Get(),
        2,
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        srvDescriptorHeap_.Get(),
        srvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart(),
        srvDescriptorHeap_->GetGPUDescriptorHandleForHeapStart()
    );
}

void DirectXCommon::InitializeFence() {
    HRESULT hr = device_->CreateFence(fenceVal_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
    assert(SUCCEEDED(hr));
    fenceEvent_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    assert(fenceEvent_ != nullptr);
}

// =========================================================================
// 描画前後処理
// =========================================================================
void DirectXCommon::PreDraw() {
    UINT bbIndex = swapChain_->GetCurrentBackBufferIndex();

    // RENDER_TARGET に遷移
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = backBuffers_[bbIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList_->ResourceBarrier(1, &barrier);

    // RTV と DSV の設定
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap_->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += static_cast<SIZE_T>(rtvDescriptorSize_) * bbIndex;
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHeap_->GetCPUDescriptorHandleForHeapStart();
    commandList_->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // 画面クリア
    FLOAT clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f }; // 背景色
    commandList_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    commandList_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    commandList_->RSSetViewports(1, &viewport_);
    commandList_->RSSetScissorRects(1, &scissorRect_);
}

void DirectXCommon::PostDraw() {
    UINT bbIndex = swapChain_->GetCurrentBackBufferIndex();

    // PRESENT に遷移
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = backBuffers_[bbIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    commandList_->ResourceBarrier(1, &barrier);

    HRESULT hr = commandList_->Close();
    assert(SUCCEEDED(hr));

    ID3D12CommandList* commandLists[] = { commandList_.Get() };
    commandQueue_->ExecuteCommandLists(1, commandLists);

    swapChain_->Present(1, 0);

    // GPUとの同期（フェンス）
    fenceVal_++;
    commandQueue_->Signal(fence_.Get(), fenceVal_);
    if (fence_->GetCompletedValue() < fenceVal_) {
        fence_->SetEventOnCompletion(fenceVal_, fenceEvent_);
        WaitForSingleObject(fenceEvent_, INFINITE);
    }

    hr = commandAllocator_->Reset();
    assert(SUCCEEDED(hr));
    hr = commandList_->Reset(commandAllocator_.Get(), nullptr);
    assert(SUCCEEDED(hr));

    // FPSの固定処理を呼ぶ
    UpdateFixFPS();
}

// =========================================================================
// ハンドル取得系
// =========================================================================
D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVCPUDescriptorHandle(uint32_t index) const {
    D3D12_CPU_DESCRIPTOR_HANDLE handle = srvDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(srvDescriptorSize_) * index;
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetSRVGPUDescriptorHandle(uint32_t index) const {
    D3D12_GPU_DESCRIPTOR_HANDLE handle = srvDescriptorHeap_->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(srvDescriptorSize_) * index;
    return handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& heap, UINT size, uint32_t index) {
    D3D12_CPU_DESCRIPTOR_HANDLE handle = heap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(size) * index;
    return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& heap, UINT size, uint32_t index) {
    D3D12_GPU_DESCRIPTOR_HANDLE handle = heap->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(size) * index;
    return handle;
}

// =========================================================================
// ユーティリティ系
// =========================================================================
DirectX::ScratchImage DirectXCommon::LoadTexture(const std::string& filePath) {
    DirectX::ScratchImage image{};
    std::wstring filePathW(filePath.begin(), filePath.end());

    HRESULT hr = DirectX::LoadFromWICFile(
        filePathW.c_str(),
        DirectX::WIC_FLAGS_FORCE_SRGB,
        nullptr,
        image);
    assert(SUCCEEDED(hr));

    DirectX::ScratchImage mipImages{};
    hr = DirectX::GenerateMipMaps(
        image.GetImages(),
        image.GetImageCount(),
        image.GetMetadata(),
        DirectX::TEX_FILTER_SRGB,
        0,
        mipImages);
    assert(SUCCEEDED(hr));

    return mipImages;
}

// =========================================================================
// FPS固定処理
// =========================================================================
void DirectXCommon::InitializeFixFPS() {
    reference_ = std::chrono::steady_clock::now();
}

void DirectXCommon::UpdateFixFPS() {
    // 1/60秒ぴったりの時間
    const std::chrono::microseconds kMinTime(static_cast<uint64_t>(1000000.0f / 60.0f));
    // 1/60秒よりわずかに短い時間
    const std::chrono::microseconds kMinCheckTime(static_cast<uint64_t>(1000000.0f / 65.0f));

    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - reference_);

    if (elapsed < kMinCheckTime) {
        while (std::chrono::steady_clock::now() - reference_ < kMinTime) {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
    reference_ = std::chrono::steady_clock::now();
}