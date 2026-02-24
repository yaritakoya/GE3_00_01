#include "Sprite.h"

void Sprite::Initialize(SpriteCommon* spriteCommon, uint32_t textureHandle) {
    assert(spriteCommon);
    spriteCommon_ = spriteCommon;

    // 1. 頂点バッファの作成（四角形なので頂点4つ）
    vertexResource_ = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * 4);
    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
    vertexBufferView_.StrideInBytes = sizeof(VertexData);
    vertexResource_->Map(0, nullptr, (void**)&vertexData_);

    // 2. インデックスバッファの作成（三角形2つで四角形を作るのでインデックス6つ）
    indexResource_ = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(uint32_t) * 6);
    indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
    indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
    indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
    indexResource_->Map(0, nullptr, (void**)&indexData_);

    // インデックスデータの書き込み（0,1,2 と 1,3,2 の三角形）
    indexData_[0] = 0; indexData_[1] = 1; indexData_[2] = 2;
    indexData_[3] = 1; indexData_[4] = 3; indexData_[5] = 2;

    // 3. マテリアルリソースの作成
    materialResource_ = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(Material));
    materialResource_->Map(0, nullptr, (void**)&materialData_);
    materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f }; // 白
    materialData_->enableLighting = false; // ライティング無効
    materialData_->uvTransform = MatrixMath::MakeIdentity4x4();

    // 4. 座標変換行列リソースの作成
    transformationMatrixResource_ = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
    transformationMatrixResource_->Map(0, nullptr, (void**)&transformationMatrixData_);
    transformationMatrixData_->WVP = MatrixMath::MakeIdentity4x4();
    transformationMatrixData_->World = MatrixMath::MakeIdentity4x4();

    // テクスチャを設定（同時にサイズなども自動設定）
    SetTexture(textureHandle);
}

void Sprite::SetTexture(uint32_t textureHandle) {
    textureHandle_ = textureHandle;

    // テクスチャの情報を取得して、スプライトのサイズを画像のサイズに合わせる
    const D3D12_RESOURCE_DESC& desc = spriteCommon_->GetTextureResourceDesc(textureHandle_);
    textureSize_ = { float(desc.Width), float(desc.Height) };
    size_ = textureSize_; // 表示サイズも画像サイズに合わせる
    textureLeftTop_ = { 0.0f, 0.0f }; // 切り出し位置リセット

    // 頂点データの更新
    AdjustTextureRect();
}

void Sprite::SetTextureRect(const Vector2& leftTop, const Vector2& size) {
    textureLeftTop_ = leftTop;
    textureSize_ = size;
    size_ = size; // 表示サイズも合わせる

    // 頂点データの更新
    AdjustTextureRect();
}

void Sprite::AdjustTextureRect() {
    // テクスチャ全体のサイズ情報を取得
    const D3D12_RESOURCE_DESC& desc = spriteCommon_->GetTextureResourceDesc(textureHandle_);
    float texWidth = float(desc.Width);
    float texHeight = float(desc.Height);

    // UV座標の計算
    float left = textureLeftTop_.x / texWidth;
    float right = (textureLeftTop_.x + textureSize_.x) / texWidth;
    float top = textureLeftTop_.y / texHeight;
    float bottom = (textureLeftTop_.y + textureSize_.y) / texHeight;

    // 左右反転
    if (isFlipX_) std::swap(left, right);
    // 上下反転
    if (isFlipY_) std::swap(top, bottom);

    // 頂点位置の計算（アンカーポイント考慮）
    float leftPos = -anchorPoint_.x * size_.x;
    float rightPos = (1.0f - anchorPoint_.x) * size_.x;
    float topPos = -anchorPoint_.y * size_.y;
    float bottomPos = (1.0f - anchorPoint_.y) * size_.y;

    // 0: 左下
    vertexData_[0].position = { leftPos,  bottomPos, 0.0f, 1.0f };
    vertexData_[0].texcoord = { left,     bottom,    0.0f, 0.0f };
    // 1: 左上
    vertexData_[1].position = { leftPos,  topPos,    0.0f, 1.0f };
    vertexData_[1].texcoord = { left,     top,       0.0f, 0.0f };
    // 2: 右下
    vertexData_[2].position = { rightPos, bottomPos, 0.0f, 1.0f };
    vertexData_[2].texcoord = { right,    bottom,    0.0f, 0.0f };
    // 3: 右上
    vertexData_[3].position = { rightPos, topPos,    0.0f, 1.0f };
    vertexData_[3].texcoord = { right,    top,       0.0f, 0.0f };
}

void Sprite::Update() {

    // ワールド行列の計算
    Matrix4x4 rotateMatrix = MatrixMath::MakeRotateZMatrix(rotation_);
    Matrix4x4 translateMatrix = MatrixMath::MakeTranslateMatrix({ position_.x, position_.y, 0.0f });
    Matrix4x4 worldMatrix = MatrixMath::Multiply(rotateMatrix, translateMatrix);

    // ビュー・プロジェクション（平行投影）
    // 画面サイズ 1280x720 を想定。原点は左上。
    Matrix4x4 viewMatrix = MatrixMath::MakeIdentity4x4();
    Matrix4x4 projectionMatrix = MatrixMath::MakeOrthographicMatrix(0.0f, 0.0f, 1280.0f, 720.0f, 0.0f, 100.0f);

    Matrix4x4 worldViewProjectionMatrix = MatrixMath::Multiply(worldMatrix, MatrixMath::Multiply(viewMatrix, projectionMatrix));

    transformationMatrixData_->World = worldMatrix;
    transformationMatrixData_->WVP = worldViewProjectionMatrix;
}

void Sprite::Draw() {

    ID3D12GraphicsCommandList* commandList = spriteCommon_->GetDxCommon()->GetCommandList();

    // 1. 頂点バッファセット
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
    commandList->IASetIndexBuffer(&indexBufferView_);

    // 2. 定数バッファセット (RootParam 0, 1)
    commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
    commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());

    // 3. テクスチャセット (RootParam 2)
    // 指定されたテクスチャのGPUハンドルを取得してセット
    commandList->SetGraphicsRootDescriptorTable(2, spriteCommon_->GetSrvHandleGPU(textureHandle_));

    // 4. 描画コマンド（6頂点 = 三角形2つ）
    commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}