#include "Input.h"
#include <assert.h>
#include <cassert>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")



void Input::Initialize(WinApp* winApp)
{
    // 借りてきたWinAppのインスタンスを記録
	this->winApp_ = winApp;

    HRESULT result;
    result = DirectInput8Create( winApp->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
    assert(SUCCEEDED(result));

    //キーボードデバイスの生成
    /*ComPtr<IDirectInputDevice8> keyboard ;*/
    result = directInput->CreateDevice(GUID_SysKeyboard, & keyboard, NULL);
    assert(SUCCEEDED(result));

    //入力データ形式のセット
    result = keyboard->SetDataFormat(&c_dfDIKeyboard); //標準形式
    assert(SUCCEEDED(result));

    //排他制御レベルのセット
    result = keyboard->SetCooperativeLevel(winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
    assert(SUCCEEDED(result));

}

void Input::Update()
{

	

    //前回のキー入力
	memcpy(keyPre, key, sizeof(key));

    // キーボード情報の取得開始
    keyboard->Acquire();

    // 全キーの入力状態を取得する
    /*BYTE key[256] = {};*/
    keyboard->GetDeviceState(sizeof(key), key);


}

bool Input::PushKey(BYTE keyNumber) {

    //指定キーをおしていればtrueを返す
    if (key[keyNumber]) {
		return true;
    }

    return false;
}

bool Input::TriggeerKey(BYTE keyNumber) {

    //指定キーをおしていればtrueを返す
    if (key[keyNumber] && !keyPre[keyNumber]) {
        return true;
    }

    return false;
}
