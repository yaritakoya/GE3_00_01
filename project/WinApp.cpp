#include "WinApp.h"
#include "externals/imgui/imgui.h"
#pragma comment(lib, "winmm.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam);

LRESULT WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}

	// メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		// ウィンドウが破棄された
	case WM_DESTROY:
		//OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	// 標準のメッセージ処理を行う
	return DefWindowProcW(hwnd, msg, wparam, lparam);

}

void WinApp::Initialize() {
	// 初期化コードをここに記述

	//// ウィンドウプロシージャ
	//wc.lpfnWndProc = WindowProc;
	//// ウィンドウクラス名(何でもよい)
	//wc.lpszClassName = L"CG2WindowClass";
	//// インスタンスバンドル
	//wc.hInstance = GetModuleHandle(nullptr);
	//// カーソル
	//wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	//// ウィンドウクラスを登録する
	//RegisterClass(&wc);

	//// クライアント領域をもとに実際のサイズにwrcを変更してもらう
	//AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//// ウィンドウの生成
	//hwnd = CreateWindow(wc.lpszClassName, // 利用するクラス名
	//	L"CG2",           // タイトルバーの文字(何でもよい)
	//	WS_OVERLAPPEDWINDOW,  // よく見るウィンドウスタイル
	//	CW_USEDEFAULT,        // 表示X座標(Windowsに任せる)
	//	CW_USEDEFAULT,        // 表示Y座標(WindowsOSに任せる)
	//	wrc.right - wrc.left, // ウィンドウ横幅
	//	wrc.bottom - wrc.top, // ウィンドウ縦幅
	//	nullptr,              // 親ウィンドウハンドル
	//	nullptr,              // メニューハンドル
	//	wc.hInstance,         // インスタンスハンドル
	//	nullptr);             // オプション

	//// ウィンドウを表示する
	//ShowWindow(hwnd_, SW_SHOW);


	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);

	//システムタイマーの分解能を上げる
	timeBeginPeriod(1);
	
	// ウィンドウプロシージャ
	wc.lpfnWndProc = WindowProc;
	// ウィンドウクラス名(何でもよい)
	wc.lpszClassName = L"CG2WindowClass";
	// インスタンスバンドル
	wc.hInstance = GetModuleHandle(nullptr);
	// カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	// ウィンドウクラスを登録する
	RegisterClass(&wc);
	// クライアント領域のサイズ
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

	// ウィンドウサイズを表す構造体体にクライアント領域を入れる
	RECT wrc = { 0, 0, kClientWidth, kClientHeight };

	// クライアント領域をもとに実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウィンドウの生成
	hwnd = CreateWindow(wc.lpszClassName, // 利用するクラス名
		L"CG2",           // タイトルバーの文字(何でもよい)
		WS_OVERLAPPEDWINDOW,  // よく見るウィンドウスタイル
		CW_USEDEFAULT,        // 表示X座標(Windowsに任せる)
		CW_USEDEFAULT,        // 表示Y座標(WindowsOSに任せる)
		wrc.right - wrc.left, // ウィンドウ横幅
		wrc.bottom - wrc.top, // ウィンドウ縦幅
		nullptr,              // 親ウィンドウハンドル
		nullptr,              // メニューハンドル
		wc.hInstance,         // インスタンスハンドル
		nullptr);             // オプション

	// ウィンドウを表示する
	ShowWindow(hwnd, SW_SHOW);
}

void WinApp::Update() {
	// 更新コードをここに記述



}

void WinApp::Finalize() {

	CloseWindow(hwnd);
	CoUninitialize();
}

bool WinApp::ProcessMessage() {
	MSG msg{};
	// Windowにメッセージが来てたら最優先で処理させる
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if(msg.message == WM_QUIT){
		return true;
	}

	return false;
}
