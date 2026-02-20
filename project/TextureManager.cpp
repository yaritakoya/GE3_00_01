#include "TextureManager.h"

TextureManager* TextureManager::instance_ = nullptr; // シングルトンインスタンスの初期化
uint32_t TextureManager::kSRVIndexTop = 1; // SRVインデックスの開始番号

TextureManager* TextureManager::GetInstance()
{
	if (instance_ == nullptr) {
		instance_ = new TextureManager();
	}
	return instance_;
}

void TextureManager::Finalize()
{
	delete instance_;
	instance_ = nullptr;
}

void TextureManager::Initialize()
{
	//SRVの数と同数
	textureDatas_.reserve(DirectXCommon::kMaxSRVCount);
}

uint32_t TextureManager::LoadTexture(const std::string& filePath)
{
    // 既に読み込み済みかチェック
    if (textureMap_.contains(filePath)) {
        return textureMap_[filePath];
    }

    assert(dxCommon_);

    // 1. ファイルから画像を読み込む
    DirectX::ScratchImage mipImages = dxCommon_->LoadTexture(filePath);
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

    // 2. テクスチャリソースを作成
    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = dxCommon_->CreateTextureResource(metadata);

    // 3. データをGPUに転送
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = dxCommon_->UploadTextureData(textureResource.Get(), mipImages);

    // リストに登録
    textureResources_.push_back(textureResource);
    intermediateResources_.push_back(intermediateResource);

    // 4. SRV（シェーダーリソースビュー）を作成
    uint32_t index = srvIndex_;
    srvIndex_++;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

    // CPUハンドルを取得してSRV作成
    dxCommon_->GetDevice()->CreateShaderResourceView(
        textureResource.Get(),&srvDesc,dxCommon_->GetSRVCPUDescriptorHandle(index));

    // マップに登録
    textureMap_[filePath] = index;

    return index;
}



