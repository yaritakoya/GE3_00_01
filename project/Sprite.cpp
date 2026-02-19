#include "Sprite.h"
#include "DirectXCommon.h"
#include "Matrix.h"

//using namespace DirectXCommon;

void Sprite::Initialize(SpriteCommon* spriteCommon) {
	// スプライトの初期化処理をここに記述
	this->spriteCommon_ = spriteCommon;

	//VertexResourceを作成
	vertexResource_ = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(VertexData) * 6);

	//IndexResourceを作成
	indexResource_ = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(uint32_t) * 6);

	//VertexBufferViewを作成
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	//IndexBufferViewを作成
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	//VertexResourceとIndexResourceをCPU側でマップして、データを書き込むためのポインタを取得
	vertexResource_->Map(0, nullptr, (void**)&vertexData_);
	//IndexResourceをマップして、データを書き込むためのアドレスを割り当てる
	indexResource_->Map(0, nullptr, (void**)&indexData_);

	//マテリアルリソースを作成
	materialResource_ = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(Vector4));
	//materialResource_ = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(Material));
	//マテリアルリソースをマップして、データを書き込むためのアドレスを割り当てる
	materialResource_->Map(0, nullptr, (void**)&materialData_);

	// マテリアルデータの初期化を書き込む
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData_->enableLighting = false;
	materialData_->uvTransform = MatrixMath::MakeIdentity4x4();

	// 座標変更行列リソースを作る
	//transformationMatrixResource_ = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(TransformationMatrix));
	transformationMatrixResource_ = spriteCommon_->GetDxCommon()->CreateBufferResource(sizeof(Matrix4x4));
	transformationMatrixResource_.Get()->Map(0, nullptr, reinterpret_cast<void**> (&transformationMatrixData_));

	// 単位行列を書き込んでおく
	transformationMatrixData_->WVP = MatrixMath::MakeIdentity4x4();
	transformationMatrixData_->World = MatrixMath::MakeIdentity4x4();
}

void Sprite::Update() {
	// 頂点リソース
	vertexData_[0].position = { 0.0f, 360.0f, 0.0f, 1.0f }; // 左下
	vertexData_[0].texcoord = { 0.0f, 1.0f };

	vertexData_[1].position = { 0.0f, 0.0f, 0.0f, 1.0f }; // 左上
	vertexData_[1].texcoord = { 0.0f, 0.0f };

	vertexData_[2].position = { 640.0f, 360.0f, 0.0f, 1.0f }; // 右下
	vertexData_[2].texcoord = { 1.0f, 1.0f };

	vertexData_[3].position = { 640.0f, 0.0f, 0.0f, 1.0f }; // 右上
	vertexData_[3].texcoord = { 1.0f, 0.0f };

	// インデックスリソース
	indexData_[0] = 0; indexData_[1] = 1; indexData_[2] = 2;
	indexData_[3] = 1; indexData_[4] = 3; indexData_[5] = 2;

	//トランスフォームの初期化
	Transform transform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	// 頂点データを書き込む
	Matrix4x4 worldMatrix = MatrixMath::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	Matrix4x4 viewMatrix = MatrixMath::MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MatrixMath::MakeOrthographicMatrix(
		0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);

	//　Transform行列の計算
	Matrix4x4 worldViewProjectionMatrix = MatrixMath::Multiply(worldMatrix, MatrixMath::Multiply(viewMatrix, projectionMatrix));

	transformationMatrixData_->WVP = worldViewProjectionMatrix;
	transformationMatrixData_->World = worldMatrix;

	Matrix4x4 uvTransformMatrix = MatrixMath::MakeScaleMatrix(uvTransform.scale);
	uvTransformMatrix = MatrixMath::Multiply(uvTransformMatrix, MatrixMath::MakeRotateZMatrix(uvTransform.rotate.z));
	uvTransformMatrix = MatrixMath::Multiply(uvTransformMatrix, MatrixMath::MakeTranslateMatrix(uvTransform.translate));
	materialData_->uvTransform = uvTransformMatrix;

	//transformationMatrixData_->WVP = MatrixMath::Multiply(worldMatrix, MatrixMath::Multiply(viewMatrix, projectionMatrix));


}

void Sprite::Draw() {

	ID3D12GraphicsCommandList* commandList = spriteCommon_->GetDxCommon()->GetCommandList();

	//vertexBufferをセット
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	//indexBufferをセット
	commandList->IASetIndexBuffer(&indexBufferView_);
	//materialリソースをセット
	commandList->SetGraphicsRootConstantBufferView(0, materialResource_.Get()->GetGPUVirtualAddress());
	//座標変換行列リソースをセット
	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_.Get()->GetGPUVirtualAddress());
	//テクスチャのSRVをセット
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = spriteCommon_->GetDxCommon()->GetSRVGPUDescriptorHandle(1);
	
	commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
	
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

}
