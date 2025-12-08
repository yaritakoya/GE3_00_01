#include "wrl.h"
#include "WinApp.h"
#define DIRECTINPUT_VERSION 0x0800// DirectInputのバージョン指定
#include <dinput.h>
#include <cassert>
#include <windows.h>

class Input
{
public://メンバ関数
	//初期化
	void Initialize(WinApp* winApp);
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
	ComPtr<IDirectInput8>directInput/* = nullptr*/;
	//キーボードのデバイス
	ComPtr<IDirectInputDevice8> keyboard;
	//キーボードの入力確認情報
	BYTE key[256] = {};
	BYTE keypre[256] = {};
	//WindowsAPI
	WinApp* winApp = nullptr;
};

