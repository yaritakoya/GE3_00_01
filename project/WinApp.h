#pragma once
#include <wrl.h>
#include <Windows.h>
#include <stdint.h>


using namespace Microsoft::WRL;

// WindowsAPI
class WinApp {
public:// 静的メンバー関数
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:
	// 初期化
	void Initialize();

	//更新
	void Update();

	// 終了
	void Finalize();

	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;
	// ウィンドウサイズを表す構造体体にクライアント領域を入れる
	RECT wrc = { 0, 0, WinApp::kClientWidth, WinApp::kClientHeight };

	//　メッセージの処理
	bool ProcessMessage();

public:
	// Getter
	HWND GetHwnd() const { return hwnd; }

	// getter
	HINSTANCE GetHInstance() const { return wc.hInstance; }
private:

	HWND hwnd= nullptr;  // ← ウィンドウハンドルを保持するメンバ変数

	WNDCLASS wc{};
};
