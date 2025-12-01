#pragma once
#include <Windows.h>
#include <cstdint>

class WinApp
{
public:
	void Initialize();

	void Update();

	//終了
	void Finalize();

	//静的メンバ関数
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	//定数
	//クライアント領域の幅
	static const int32_t kClientWidth = 1280;
	//クライアント領域の高さ
	static const int32_t kClientHeight = 720;
	//getter
	HWND GetHwnd() { return hwnd; }
	HINSTANCE GetHInstance() { return wc.hInstance; }
	//ウィンドウクラスの設定
	WNDCLASS wc{};

private:
	//ウィンドウハンドル
	HWND hwnd = nullptr;
};

