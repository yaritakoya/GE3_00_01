#include "wrl.h"
#define DIRECTINPUT_VERSION 0x0800// DirectInputのバージョン指定
#include <dinput.h>
#include <cassert>
#include <windows.h>

#pragma comment(lib,"dinput8.lib")// DirectInputのライブラリをリンクする
#pragma comment(lib,"dxguid.lib")// DirectInputで必要になるライブラリをリンクする

class Input
{
public://メンバ関数
	//初期化
	void Initialize(HINSTANCE hInstance,HWND hwnd);
	//更新
	void Update();
	//namespace省略
	template<class T>using ComPtr = Microsoft::WRL::ComPtr<T>;

	//現在の全キーの状態
	bool PushKey(BYTE keyNumber);
	//前回の全キーの状態
	bool Triggerkey(BYTE keyNumber);

private://メンバ変数
	//DirectInput8のインターフェース
	ComPtr<IDirectInput8>directInput = nullptr;
	//キーボードのデバイス
	ComPtr<IDirectInputDevice8> keyboard;
	//キーボードの入力確認情報
	BYTE key[256] = {};
	BYTE keypre[256] = {};
};

