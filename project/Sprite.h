#pragma once
#include "SpriteCommon.h"
#include "Matrix.h"

class Sprite {
public:
    // 初期化（テクスチャ番号を指定）
    void Initialize(SpriteCommon* spriteCommon, uint32_t textureHandle = 0);

    // 更新
    void Update();

    // 描画
    void Draw();

    // --- ゲッター・セッター ---

    // 座標
    const Vector2& GetPosition() const { return position_; }
    void SetPosition(const Vector2& position) { position_ = position; }

    // 回転 (これが無いとエラーになります！)
    float GetRotation() const { return rotation_; }
    void SetRotation(float rotation) { rotation_ = rotation; }

    // サイズ（通常はテクスチャサイズが自動設定されますが、手動変更も可能）
    const Vector2& GetSize() const { return size_; }
    void SetSize(const Vector2& size) { size_ = size; }

    // 色
    const Vector4& GetColor() const { return materialData_->color; }
    void SetColor(const Vector4& color) { materialData_->color = color; }

    // テクスチャ変更
    void SetTexture(uint32_t textureHandle);

    // テクスチャ切り出し範囲（左上座標、サイズ）
    void SetTextureRect(const Vector2& leftTop, const Vector2& size);

    // アンカーポイント（0.5, 0.5で中心、0,0で左上）
    void SetAnchorPoint(const Vector2& anchorPoint) { anchorPoint_ = anchorPoint; }

    // 左右・上下反転
    void SetFlip(bool isFlipX, bool isFlipY) { isFlipX_ = isFlipX; isFlipY_ = isFlipY; }

private:
    // 頂点データの更新（サイズや切り取り範囲が変わったら呼ぶ）
    void AdjustTextureRect();

private:
    SpriteCommon* spriteCommon_ = nullptr;

    struct VertexData {
        Vector4 position;
        Vector4 texcoord;
    };

    struct Material {
        Vector4 color;
        int32_t enableLighting;
        float padding[3];
        Matrix4x4 uvTransform;
    };

    struct TransformationMatrix {
        Matrix4x4 WVP;
        Matrix4x4 World;
    };

    // バッファリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
    VertexData* vertexData_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
    D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
    uint32_t* indexData_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    Material* materialData_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
    TransformationMatrix* transformationMatrixData_ = nullptr;

    Transform uvTransform_{ {1,1,1}, {0,0,0}, {0,0,0} };

    // スプライトパラメータ
    Vector2 position_ = { 0.0f, 0.0f };
    float rotation_ = 0.0f;
    Vector2 size_ = { 100.0f, 100.0f };
    Vector2 anchorPoint_ = { 0.0f, 0.0f }; // デフォルトは左上
    bool isFlipX_ = false;
    bool isFlipY_ = false;

    // テクスチャ関連
    uint32_t textureHandle_ = 0; // 使っているテクスチャの番号
    Vector2 textureLeftTop_ = { 0.0f, 0.0f }; // テクスチャの左上オフセット
    Vector2 textureSize_ = { 100.0f, 100.0f }; // テクスチャの切り出しサイズ
};