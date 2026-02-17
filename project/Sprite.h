#pragma once  
#include "SpriteCommon.h"  
#include "DirectXCommon.h"
#include <Matrix.h>

class Sprite {
public:
	void Initialize(SpriteCommon * spriteCommon);

	void Update();

	void Draw();

private:
	SpriteCommon * spriteCommon_; // メンバー変数を追加  

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

	// 変換行列データ
    struct TransformationMatrix {
        Matrix4x4 WVP;
        Matrix4x4 World;
    };

	

    Transform uvTransform{
        {1.0f, 1.0f, 1.0f},
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 0.0f}
    };

	// バッファリソース
       // ▼ 頂点バッファ（VertexBuffer）の GPU リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;

    // ▼ インデックスバッファ（IndexBuffer）の GPU リソース
    Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;

    // ▼ CPU 側のデータ（頂点配列）
    VertexData* vertexData_ = nullptr;

    // ▼ CPU 側のデータ（インデックス配列）
    uint32_t* indexData_ = nullptr;

    // ▼ GPU に渡す際に必要なビュー構造体
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
    D3D12_INDEX_BUFFER_VIEW indexBufferView_;


    // マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    // バッファリソース内のデータを指すポインタ
	Material* materialData_ = nullptr;


    // バッファリソース
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* transformationMatrixData_ = nullptr;

    DirectXCommon* dxCommon_ = nullptr;

};
