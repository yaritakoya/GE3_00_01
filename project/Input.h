#pragma once
#include <Windows.h>
#define DIRECTINPUT_VERSION 0x0800  // DirectInputのバージョン指定
#include <dinput.h> //DirectInput
#include <wrl.h>
#include "WinApp.h"

using namespace Microsoft::WRL;


//入力
class Input {

public:
	//namespace省略
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	//初期化
	void Initialize(WinApp * winApp);
	//更新
	void Update();

	/// <summary>
	/// キーを押下をチェック
	/// </summary>
	/// <param name="keyNumber">キー番号</param>
	/// <returns>押されているか</returns>
	bool PushKey(BYTE keyNumber); //キーが押された瞬間


	/// <summary>
	/// キーのトリガーをチェック
	/// </summary>
	/// <param name="keyNumber">キー番号</param>
	/// <returns>トリガーか</returns>
	bool TriggeerKey(BYTE keyNumber); 

private:
	//キーボードデバイス
	ComPtr<IDirectInputDevice8> keyboard;
	
	//DirectInputのインスタンスの生成
	ComPtr<IDirectInput8> directInput = nullptr;

	//全キーの状態
	BYTE keyPre[256] = {};

	BYTE key[256] = {};
	
	//WindowsAPI
	WinApp* winApp_ = nullptr;

};

