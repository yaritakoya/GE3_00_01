#pragma once
#include <string>
#include <map>
#include "DirectXCommon.h"
#include "SpriteCommon.h"

//テクスチャマネージャー
class TextureManager {
public:
	//シングルトンインスタンスを取得する関数
	static TextureManager* GetInstance();
	//終了
	void Finalize();
	//初期化
	void Initialize();
	//テクスチャ読み込み
	uint32_t LoadTexture(const std::string& filePath);
	//SRVインデックスの開始番号
	uint32_t GetTextureIndexByFilePath(const std::string& filePath);
	static uint32_t kSRVIndexTop;

private:
	static TextureManager* instance_; // シングルトンインスタンス

	TextureManager() = default; // コンストラクタをプライベートにする
	~TextureManager() = default; // デストラクタをデフォルトにする
	TextureManager(const TextureManager&) = delete; // コピーコンストラクタを削除
	TextureManager& operator=(const TextureManager&) = delete; // コピー代入演算子を削除

	//テクスチャ1枚分のデータ
	struct TextureData {
		std::string filePath; //テクスチャのファイルパス
		DirectX::TexMetadata metadata; //テクスチャのメタデータ	
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU; //SRVハンドル（CPU）
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU; //SRVハンドル（GPU）
	};
	//テクスチャデータ
	std::vector<TextureData> textureDatas_;
	//
	std::map<std::string, uint32_t> textureMap_;
	//DirectXCommon のポインタ
	DirectXCommon* dxCommon_ = nullptr;
	// テクスチャリソースの配列
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResources_;
	// データ転送用の中間リソースの配列
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResources_;
	// SRV ハンドルのインデックス
	uint32_t srvIndex_ = 0;
};