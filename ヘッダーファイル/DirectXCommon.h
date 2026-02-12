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
	// 初期化
	void Initialize(WinApp* winApp);
	
	// 描画前後
	void PreDraw();
	void PostDraw();

	// SRV ハンドル取得
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index) const;

	// getter（資料のスライド）
	ID3D12DescriptorHeap* GetSRVHeap()	const { return srvDescriptorHeap_.Get(); }

private:

	// D3D12デバイスを生成する
	void InitializeDevice();

	// コマンドキュー／コマンドアロケータ／コマンドリストを生成する
	void InitializeCommand();

	// スワップチェーンを生成する（画面表示用バッファ）
	void InitializeSwapChain(WinApp* winApp);

	// 各種ディスクリプタヒープを生成する（RTV / DSV / SRV など）
	void InitializeDescriptorHeaps();

	// レンダーターゲットビュー（RTV）を生成する
	void InitializeRenderTargetView();

	// 深度バッファ用リソースを生成する
	void InitializeDepthBuffer();

	// 深度ステンシルビュー（DSV）を生成する
	void InitializeDepthStencilView();

	// ビューポートを設定する（描画範囲）
	void InitializeViewport();

	// シザー矩形を設定する（描画制限領域）
	void InitializeScissorRect();

	// DXCコンパイラを初期化する（HLSLコンパイル用）
	void InitializeDXCCompiler();

	// ImGuiを初期化する
	void InitializeImGui();

	// フェンスを生成する（GPU同期用）
	void InitializeFence();

private:
	// Windowsアプリケーション
	WinApp* winApp_ = nullptr;

	// デバイス関連
	ComPtr<ID3D12Device> device_;
	ComPtr<IDXGIFactory7> dxgiFactory_;
	ComPtr<IDXGIAdapter4> adapter_;

	// コマンド関連
	ComPtr<ID3D12CommandQueue>        commandQueue_;
	ComPtr<ID3D12CommandAllocator>    commandAllocator_;
	ComPtr<ID3D12GraphicsCommandList> commandList_;

	// RTV 
	ComPtr<ID3D12DescriptorHeap> rtvHeap_;
	UINT rtvDescriptorSize_ = 0;

	// DSV
	ComPtr<ID3D12DescriptorHeap> dsvHeap_;
	ComPtr<ID3D12Resource> depthBuffer_;
	UINT dsvDescriptorSize_ = 0;

	// SRV
	ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
	UINT srvDescriptorSize_ = 0;

	// フェンス
	ComPtr<ID3D12Fence> fence_;
	UINT64 fenceVal_ = 0;
	HANDLE fenceEvent_ = nullptr;

	ComPtr<IDXGISwapChain4> swapChain_; // スワップチェーン
	std::array<ComPtr<ID3D12Resource>, 2> backBuffers_; // バックバッファ2枚
	D3D12_VIEWPORT viewport_{}; // ビューポート
	D3D12_RECT     scissorRect_{}; //シザー

	// DXC
	ComPtr<IDxcUtils>          dxcUtils_;
	ComPtr<IDxcCompiler3>      dxcCompiler_;
	ComPtr<IDxcIncludeHandler> includeHandler_;

	// 任意のインデックスのハンドルを計算するヘルパ
	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(
		const ComPtr<ID3D12DescriptorHeap>& heap,
		UINT size, uint32_t index);

	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(
		const ComPtr<ID3D12DescriptorHeap>& heap,
		UINT size, uint32_t index);
};