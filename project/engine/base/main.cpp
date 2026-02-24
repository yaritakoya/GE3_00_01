#include <Windows.h>
#include "WinApp.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "SpriteCommon.h"
#include "Sprite.h"

// ImGuiなど...

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    WinApp* winApp = new WinApp();
    winApp->Initialize();
    DirectXCommon* dxCommon = new DirectXCommon();
    dxCommon->Initialize(winApp);
    Input* input = new Input();
    input->Initialize(winApp);

    // --- SpriteCommon 初期化 ---
    SpriteCommon* spriteCommon = new SpriteCommon();
    spriteCommon->Initialize(dxCommon);

    // --- テクスチャ読み込み（ここで一括管理！） ---
    // LoadTextureはテクスチャハンドル（番号）を返す
    uint32_t textureHandleMonster = spriteCommon->LoadTexture("Resources/uvChecker.png");
    uint32_t textureHandleTitle = spriteCommon->LoadTexture("Resources/uvChecker.png");

    // --- スプライト生成 ---
    // 1つ目：モンスターボール（そのまま表示）
    Sprite* sprite1 = new Sprite();
    sprite1->Initialize(spriteCommon, textureHandleMonster); // ハンドルを指定して初期化
    sprite1->SetPosition({ 300.0f, 360.0f }); // 座標指定

    // 2つ目：タイトル画像（一部を切り取り表示）
    Sprite* sprite2 = new Sprite();
    sprite2->Initialize(spriteCommon, textureHandleTitle);
    sprite2->SetPosition({ 800.0f, 360.0f });
    sprite2->SetTextureRect({ 0.0f, 0.0f }, { 200.0f, 100.0f }); // 左上から200x100だけ切り取る
    sprite2->SetAnchorPoint({ 0.5f, 0.5f }); // 中心を原点にする

    while (true) {
        if (winApp->ProcessMessage()) break;
        input->Update();
        if (input->PushKey(DIK_ESCAPE)) break;

        // --- 更新 ---
        sprite1->SetRotation(sprite1->GetRotation() + 0.02f); // クルクル回してみる
        sprite1->Update();

        sprite2->Update();

        // --- 描画 ---
        dxCommon->PreDraw();
        spriteCommon->PreDraw(); // 共通設定ON

        sprite1->Draw();
        sprite2->Draw();

        dxCommon->PostDraw();
    }

    delete sprite1;
    delete sprite2;
    delete spriteCommon;
    delete input;
    delete dxCommon;
    delete winApp;

    return 0;
}