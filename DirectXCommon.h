#pragma once

#include "WinApp.h"

#include <array>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <wrl.h>
#include <string>

#include "externals/DirectXTex/DirectXTex.h"

using Microsoft::WRL::ComPtr;

class DirectXCommon {
public:
	// 初期化 / 描画前後
	void Initialize(WinApp* winApp);
	void PreDraw();
	void PostDraw();

	// SRV ハンドル取得
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index) const;

	// getter（資料のスライド）
	ID3D12DescriptorHeap* GetSRVHeap()     const { return srvDescriptorHeap_.Get(); }

private:
	// 内部初期化（スライドの「初期化処理のコメント解除」で使うやつ）
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

private:
	WinApp* winApp_ = nullptr;

	// デバイスまわり
	ComPtr<ID3D12Device> device_;
	ComPtr<IDXGIFactory7> dxgiFactory_;
	ComPtr<IDXGIAdapter4> adapter_;

	// コマンドまわり
	ComPtr<ID3D12CommandQueue>        commandQueue_;
	ComPtr<ID3D12CommandAllocator>    commandAllocator_;
	ComPtr<ID3D12GraphicsCommandList> commandList_;

	// スワップチェーン & バックバッファ
	ComPtr<IDXGISwapChain4> swapChain_;
	std::array<ComPtr<ID3D12Resource>, 2> backBuffers_;

	// RTV / DSV
	ComPtr<ID3D12DescriptorHeap> rtvHeap_;
	UINT rtvDescriptorSize_ = 0;

	ComPtr<ID3D12DescriptorHeap> dsvHeap_;
	ComPtr<ID3D12Resource> depthBuffer_;
	UINT dsvDescriptorSize_ = 0;

	// SRV 用ヒープ
	ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
	UINT srvDescriptorSize_ = 0;

	// フェンス
	ComPtr<ID3D12Fence> fence_;
	UINT64 fenceVal_ = 0;
	HANDLE fenceEvent_ = nullptr;

	// ビューポート / シザー
	D3D12_VIEWPORT viewport_{};
	D3D12_RECT     scissorRect_{};

	// DXC
	ComPtr<IDxcUtils>          dxcUtils_;
	ComPtr<IDxcCompiler3>      dxcCompiler_;
	ComPtr<IDxcIncludeHandler> includeHandler_;

	// 任意のインデックスのハンドルを計算するヘルパ
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(
		const ComPtr<ID3D12DescriptorHeap>& heap,
		UINT size,
		uint32_t index);

	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(
		const ComPtr<ID3D12DescriptorHeap>& heap,
		UINT size,
		uint32_t index);
};