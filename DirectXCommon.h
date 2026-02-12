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

using Microsoft::WRL::ComPtr;

class DirectXCommon {
public: // ★ ここから下の関数はどこからでもアクセスできる (public) ★

	// 初期化
	void Initialize(WinApp* winApp);

	// 描画前後
	void PreDraw();
	void PostDraw();

	// SRV ハンドル取得
	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index) const;

	// ====================================================
	// ★ ここに Getter をまとめる（絶対に private の上に書く）
	// ====================================================
	ID3D12Device* GetDevice() const { return device_.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }
	ID3D12DescriptorHeap* GetSRVHeap() const { return srvDescriptorHeap_.Get(); }


private: // ★ ここから下の変数は外部からアクセスできない (private) ★

	// D3D12デバイスを生成する
	void InitializeDevice();

	// コマンドキュー／コマンドアロケータ／コマンドリストを生成する
	void InitializeCommand();

	// スワップチェーンを生成する（画面表示用バッファ）
	void InitializeSwapChain(WinApp* winApp);

	// 各種ディスクリプタヒープを生成する
	void InitializeDescriptorHeaps();

	// レンダーターゲットビューを生成する
	void InitializeRenderTargetView();

	// 深度バッファを生成する
	void InitializeDepthBuffer();

	// 深度ステンシルビューを生成する
	void InitializeDepthStencilView();

	// ビューポートを初期化する
	void InitializeViewport();

	// シザー矩形を初期化する
	void InitializeScissorRect();

	// DXCコンパイラを初期化する
	void InitializeDXCCompiler();

	// FPS 固定初期化
	void InitializeFixFPS();

	// FPS 固定更新
	void UpdateFixFPS();

	// ImGuiを初期化する
	void InitializeImGui();

	// フェンスを生成する（GPU同期用）
	void InitializeFence();

	// 任意のインデックスのハンドルを計算するヘルパ
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& heap, UINT size, uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& heap, UINT size, uint32_t index);

	// テクスチャ読み込み
	DirectX::ScratchImage LoadTexture(const std::string& filePath);

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

	ComPtr<IDXGISwapChain4> swapChain_;
	std::array<ComPtr<ID3D12Resource>, 2> backBuffers_;

	D3D12_VIEWPORT viewport_{};
	D3D12_RECT scissorRect_{};

	// 記録時間(FPS固定用)
	std::chrono::steady_clock::time_point reference_;
};