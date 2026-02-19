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

};
