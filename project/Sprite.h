#pragma once  
#include "SpriteCommon.h"  
#include "DirectXCommon.h"
#include <Matrix.h>

class SpriteCommon;

class Sprite {
public:
	void Initialize(SpriteCommon* spriteCommon);

	void Update();

	void Draw();

	//getter/setter
	//▼スプライトの位置を取得・設定する関数
	const Vector2& GetPosition() const { return position_; }
	void SetPosition(const Vector2& position) { position_ = position; }
	//▼スプライトの回転を取得・設定する関数
	float GetRotation() const { return rotation_; }
	void SetRotation(float rotation) { rotation_ = rotation; }
	//▼スプライトの色を取得・設定する関数
	const Vector4& GetColor() const { return materialData_->color; }
	void SetColor(const Vector4& color) { materialData_->color = color; }
	//▼スプライトのサイズを取得・設定する関数
	const Vector2& GetSize() const { return size_; }
	void SetSize(const Vector2& size) { size_ = size; }
	
private:
	SpriteCommon* spriteCommon_; // メンバー変数を追加  

	// 頂点データ
	struct VertexData {
		Vector4 position; // 位置
		Vector4 texcoord; // テクスチャ座標
		//Vector3 normal;   // 法線
	};

	// マテリアルデータ
	struct Material {
		Vector4 color;
		int32_t enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};

	// 座標変換行列データ
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};

	// UV 変換行列の初期値（単位行列）
	Transform uvTransform{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};

	// ▼バッファリソース	
	ComPtr<ID3D12Resource> vertexResource_;	//  VertexBuffer リソース
	ComPtr<ID3D12Resource> indexResource_;	// IndexBuffer リソース
	ComPtr<ID3D12Resource> materialResource_;	// マテリアルリソース
	ComPtr<ID3D12Resource> transformationMatrixResource_;	// 座標変換行列リソース

	// ▼ バッファリソース内のデータを指すポインタ
	VertexData* vertexData_ = nullptr;	// 頂点データへのポインタ
	uint32_t* indexData_ = nullptr;	// インデックスデータへのポインタ
	Material* materialData_ = nullptr;	// マテリアルデータへのポインタ
	TransformationMatrix* transformationMatrixData_ = nullptr;	// 座標変換行列データへのポインタ

	// バッファリソースの使い方を補足するバッファビュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_;

	//
	DirectXCommon* dxCommon_ = nullptr;

	//sprite座標
	Vector2 position_ = { 0.0f, 0.0f };
	float rotation_ = 0.0f;
	Vector2 size_ = { 640.0f, 360.0f };

};
