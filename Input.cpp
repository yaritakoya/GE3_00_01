#include "Input.h"

void Input::Initialize(HINSTANCE hInstance,HWND hwnd)
{
	HRESULT result;
	//Direct Input の初期化
	//IDirectInput8* directInput = nullptr;
	result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(result));
	// キーボードデバイスの生成
	//IDirectInputDevice8* keyboard = nullptr;
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));
	//入力データ形式のセット
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));
	//排他制御レベルのセット
	result = keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

void Input::Update()
{
	//前回のキー情報を保存
	memcpy(keypre, key, sizeof(key));
	//キーボード情報の取得開始
	keyboard->Acquire();
	//キーボードの入力情報を得る
	keyboard->GetDeviceState(sizeof(key), key);
}

bool Input::PushKey(BYTE keyNumber)
{
	//指定キーを押していればtrueを返す
	if (key[keyNumber]) {
		return true;
	}
	return false;
}

bool Input::Triggerkey(BYTE keyNumber)
{
	//指定キーを押していればtrueを返す
	if (key[keyNumber] && !keypre[keyNumber]) {
		return true;
	}
	return false;
}
