// --- 定数定義 ---
#define _USE_MATH_DEFINES

// --- Windows / 標準ライブラリ ---
#include <Windows.h>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <strsafe.h>
#include <vector>
#include <format>     
#include <sstream>
#include <iostream>

#include "Input.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "SpriteCommon.h"   
#include "Sprite.h"         

// --- Direct3D 12 / DXGI 関連 ---
#include <d3d12.h>
#include <dxgi1_6.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

// --- DirectX デバッグ支援 ---
#include <dbghelp.h>
#include <dxgidebug.h>
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "dxguid.lib")

// --- DXC (Shader Compiler) ---
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")

// --- DirectXTex ---
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h" // d3dx12.h はヘッダのみ

// --- ImGui ---
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

// --- XAudio2 ---
#include <xaudio2.h>
#pragma comment(lib, "xaudio2.lib")

// --- DirectInput ---
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <ctime> 

// ImGui の WndProc ハンドラ
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ---------- 基本構造体 ----------
//struct Vector2 {
//    float x, y;
//};
//
//struct Vector3 {
//    float x, y, z;
//};
//
//struct Vector4 {
//    float x, y, z, w;
//};


struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct Fragment {
	Vector3 position;
	Vector3 velocity;
	Vector3 rotation;
	Vector3 rotationSpeed;
	float alpha;
	bool active;
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

struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

// CG2_06_02
struct MaterialData {
	std::string textureFilePath;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

// --- 列挙体 ---
enum WaveType {
	WAVE_SINE,
	WAVE_CHAINSAW,
	WAVE_SQUARE,
};

enum AnimationType {
	ANIM_RESET,
	ANIM_NONE,
	ANIM_COLOR,
	ANIM_SCALE,
	ANIM_ROTATE,
	ANIM_TRANSLATE,
	ANIM_TORNADO,
	ANIM_PULSE,
	ANIM_AURORA,
	ANIM_BOUNCE,
	ANIM_TWIST,
	ANIM_ALL
};

// 16分割（球生成用）
const int kSubdivision = 16;
int kNumVertices = kSubdivision * kSubdivision * 6;

WaveType waveType = WAVE_SINE;
AnimationType animationType = ANIM_NONE;
float waveTime = 0.0f;

// --- グローバルポインタ ---
Input* input = nullptr;
WinApp* winApp = nullptr;
DirectXCommon* dxCommon = nullptr;
SpriteCommon* spriteCommon = nullptr;
Sprite* sprite = nullptr;

// ===============================
//  行列・数学系の関数
// ===============================

#pragma region 行列関数

//Matrix4x4 MakeIdentity4x4() {
//    Matrix4x4 result{};
//    for (int i = 0; i < 4; ++i) {
//        result.m[i][i] = 1.0f;
//    }
//    return result;
//}
//
//Matrix4x4 Matrix4x4MakeScaleMatrix(const Vector3& s) {
//    Matrix4x4 result = {};
//    result.m[0][0] = s.x;
//    result.m[1][1] = s.y;
//    result.m[2][2] = s.z;
//    result.m[3][3] = 1.0f;
//    return result;
//}
//
//Matrix4x4 MakeRotateXMatrix(float radian) {
//    Matrix4x4 result = {};
//
//    result.m[0][0] = 1.0f;
//    result.m[1][1] = std::cos(radian);
//    result.m[1][2] = std::sin(radian);
//    result.m[2][1] = -std::sin(radian);
//    result.m[2][2] = std::cos(radian);
//    result.m[3][3] = 1.0f;
//
//    return result;
//}
//
//Matrix4x4 MakeRotateYMatrix(float radian) {
//    Matrix4x4 result = {};
//
//    result.m[0][0] = std::cos(radian);
//    result.m[0][2] = std::sin(radian);
//    result.m[1][1] = 1.0f;
//    result.m[2][0] = -std::sin(radian);
//    result.m[2][2] = std::cos(radian);
//    result.m[3][3] = 1.0f;
//
//    return result;
//}
//
//Matrix4x4 MakeRotateZMatrix(float radian) {
//    Matrix4x4 result = {};
//
//    result.m[0][0] = std::cos(radian);
//    result.m[0][1] = -std::sin(radian);
//    result.m[1][0] = std::sin(radian);
//    result.m[1][1] = std::cos(radian);
//    result.m[2][2] = 1.0f;
//    result.m[3][3] = 1.0f;
//
//    return result;
//}
//
//Matrix4x4 MakeTranslateMatrix(const Vector3& tlanslate) {
//    Matrix4x4 result = {};
//    result.m[0][0] = 1.0f;
//    result.m[1][1] = 1.0f;
//    result.m[2][2] = 1.0f;
//    result.m[3][3] = 1.0f;
//    result.m[3][0] = tlanslate.x;
//    result.m[3][1] = tlanslate.y;
//    result.m[3][2] = tlanslate.z;
//    return result;
//}
//
//Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
//    Matrix4x4 result{};
//    for (int i = 0; i < 4; ++i) {
//        for (int j = 0; j < 4; ++j) {
//            for (int k = 0; k < 4; ++k) {
//                result.m[i][j] += m1.m[i][k] * m2.m[k][j];
//            }
//        }
//    }
//    return result;
//}
//
//Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
//    Matrix4x4 scaleMatrix = Matrix4x4MakeScaleMatrix(scale);
//    Matrix4x4 rotateX = MakeRotateXMatrix(rotate.x);
//    Matrix4x4 rotateY = MakeRotateYMatrix(rotate.y);
//    Matrix4x4 rotateZ = MakeRotateZMatrix(rotate.z);
//    Matrix4x4 rotateMatrix = Multiply(Multiply(rotateX, rotateY), rotateZ);
//    Matrix4x4 translateMatrix = MakeTranslateMatrix(translate);
//
//    Matrix4x4 worldMatrix = Multiply(Multiply(scaleMatrix, rotateMatrix), translateMatrix);
//    return worldMatrix;
//}
//
//Matrix4x4 Inverse(Matrix4x4 m) {
//    Matrix4x4 result;
//    float det;
//    int i;
//
//    result.m[0][0] =
//        m.m[1][1] * m.m[2][2] * m.m[3][3] - m.m[1][1] * m.m[2][3] * m.m[3][2] -
//        m.m[2][1] * m.m[1][2] * m.m[3][3] + m.m[2][1] * m.m[1][3] * m.m[3][2] +
//        m.m[3][1] * m.m[1][2] * m.m[2][3] - m.m[3][1] * m.m[1][3] * m.m[2][2];
//
//    result.m[0][1] =
//        -m.m[0][1] * m.m[2][2] * m.m[3][3] + m.m[0][1] * m.m[2][3] * m.m[3][2] +
//        m.m[2][1] * m.m[0][2] * m.m[3][3] - m.m[2][1] * m.m[0][3] * m.m[3][2] -
//        m.m[3][1] * m.m[0][2] * m.m[2][3] + m.m[3][1] * m.m[0][3] * m.m[2][2];
//
//    result.m[0][2] =
//        m.m[0][1] * m.m[1][2] * m.m[3][3] - m.m[0][1] * m.m[1][3] * m.m[3][2] -
//        m.m[1][1] * m.m[0][2] * m.m[3][3] + m.m[1][1] * m.m[0][3] * m.m[3][2] +
//        m.m[3][1] * m.m[0][2] * m.m[1][3] - m.m[3][1] * m.m[0][3] * m.m[1][2];
//
//    result.m[0][3] =
//        -m.m[0][1] * m.m[1][2] * m.m[2][3] + m.m[0][1] * m.m[1][3] * m.m[2][2] +
//        m.m[1][1] * m.m[0][2] * m.m[2][3] - m.m[1][1] * m.m[0][3] * m.m[2][2] -
//        m.m[2][1] * m.m[0][2] * m.m[1][3] + m.m[2][1] * m.m[0][3] * m.m[1][2];
//
//    result.m[1][0] =
//        -m.m[1][0] * m.m[2][2] * m.m[3][3] + m.m[1][0] * m.m[2][3] * m.m[3][2] +
//        m.m[2][0] * m.m[1][2] * m.m[3][3] - m.m[2][0] * m.m[1][3] * m.m[3][2] -
//        m.m[3][0] * m.m[1][2] * m.m[2][3] + m.m[3][0] * m.m[1][3] * m.m[2][2];
//
//    result.m[1][1] =
//        m.m[0][0] * m.m[2][2] * m.m[3][3] - m.m[0][0] * m.m[2][3] * m.m[3][2] -
//        m.m[2][0] * m.m[0][2] * m.m[3][3] + m.m[2][0] * m.m[0][3] * m.m[3][2] +
//        m.m[3][0] * m.m[0][2] * m.m[2][3] - m.m[3][0] * m.m[0][3] * m.m[2][2];
//
//    result.m[1][2] =
//        -m.m[0][0] * m.m[1][2] * m.m[3][3] + m.m[0][0] * m.m[1][3] * m.m[3][2] +
//        m.m[1][0] * m.m[0][2] * m.m[3][3] - m.m[1][0] * m.m[0][3] * m.m[3][2] -
//        m.m[3][0] * m.m[0][2] * m.m[1][3] + m.m[3][0] * m.m[0][3] * m.m[1][2];
//
//    result.m[1][3] =
//        m.m[0][0] * m.m[1][2] * m.m[2][3] - m.m[0][0] * m.m[1][3] * m.m[2][2] -
//        m.m[1][0] * m.m[0][2] * m.m[2][3] + m.m[1][0] * m.m[0][3] * m.m[2][2] +
//        m.m[2][0] * m.m[0][2] * m.m[1][3] - m.m[2][0] * m.m[0][3] * m.m[1][2];
//
//    result.m[2][0] =
//        m.m[1][0] * m.m[2][1] * m.m[3][3] - m.m[1][0] * m.m[2][3] * m.m[3][1] -
//        m.m[2][0] * m.m[1][1] * m.m[3][3] + m.m[2][0] * m.m[1][3] * m.m[3][1] +
//        m.m[3][0] * m.m[1][1] * m.m[2][3] - m.m[3][0] * m.m[1][3] * m.m[2][1];
//
//    result.m[2][1] =
//        -m.m[0][0] * m.m[2][1] * m.m[3][3] + m.m[0][0] * m.m[2][3] * m.m[3][1] +
//        m.m[2][0] * m.m[0][1] * m.m[3][3] - m.m[2][0] * m.m[0][3] * m.m[3][1] -
//        m.m[3][0] * m.m[0][1] * m.m[2][3] + m.m[3][0] * m.m[0][3] * m.m[2][1];
//
//    result.m[2][2] =
//        m.m[0][0] * m.m[1][1] * m.m[3][3] - m.m[0][0] * m.m[1][3] * m.m[3][1] -
//        m.m[1][0] * m.m[0][1] * m.m[3][3] + m.m[1][0] * m.m[0][3] * m.m[3][1] +
//        m.m[3][0] * m.m[0][1] * m.m[1][3] - m.m[3][0] * m.m[0][3] * m.m[1][1];
//
//    result.m[2][3] =
//        -m.m[0][0] * m.m[1][1] * m.m[2][3] + m.m[0][0] * m.m[1][3] * m.m[2][1] +
//        m.m[1][0] * m.m[0][1] * m.m[2][3] - m.m[1][0] * m.m[0][3] * m.m[2][1] -
//        m.m[2][0] * m.m[0][1] * m.m[1][3] + m.m[2][0] * m.m[0][3] * m.m[1][1];
//
//    result.m[3][0] =
//        -m.m[1][0] * m.m[2][1] * m.m[3][2] + m.m[1][0] * m.m[2][2] * m.m[3][1] +
//        m.m[2][0] * m.m[1][1] * m.m[3][2] - m.m[2][0] * m.m[1][2] * m.m[3][1] -
//        m.m[3][0] * m.m[1][1] * m.m[2][2] + m.m[3][0] * m.m[1][2] * m.m[2][1];
//
//    result.m[3][1] =
//        m.m[0][0] * m.m[2][1] * m.m[3][2] - m.m[0][0] * m.m[2][2] * m.m[3][1] -
//        m.m[2][0] * m.m[0][1] * m.m[3][2] + m.m[2][0] * m.m[0][2] * m.m[3][1] +
//        m.m[3][0] * m.m[0][1] * m.m[2][2] - m.m[3][0] * m.m[0][2] * m.m[2][1];
//
//    result.m[3][2] =
//        -m.m[0][0] * m.m[1][1] * m.m[3][2] + m.m[0][0] * m.m[1][2] * m.m[3][1] +
//        m.m[1][0] * m.m[0][1] * m.m[3][2] - m.m[1][0] * m.m[0][2] * m.m[3][1] -
//        m.m[3][0] * m.m[0][1] * m.m[1][2] + m.m[3][0] * m.m[0][2] * m.m[1][1];
//
//    result.m[3][3] =
//        m.m[0][0] * m.m[1][1] * m.m[2][2] - m.m[0][0] * m.m[1][2] * m.m[2][1] -
//        m.m[1][0] * m.m[0][1] * m.m[2][2] + m.m[1][0] * m.m[0][2] * m.m[2][1] +
//        m.m[2][0] * m.m[0][1] * m.m[1][2] - m.m[2][0] * m.m[0][2] * m.m[1][1];
//
//    det = m.m[0][0] * result.m[0][0] + m.m[0][1] * result.m[1][0] +
//        m.m[0][2] * result.m[2][0] + m.m[0][3] * result.m[3][0];
//
//    if (det == 0.0f) {
//        return Matrix4x4{}; // ゼロ行列返しておく
//    }
//
//    det = 1.0f / det;
//
//    for (i = 0; i < 4; i++) {
//        for (int j = 0; j < 4; j++) {
//            result.m[i][j] *= det;
//        }
//    }
//
//    return result;
//}
//
//Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio,
//    float nearClip, float farClip) {
//    Matrix4x4 result = {};
//
//    float f = 1.0f / std::tan(fovY / 2.0f);
//
//    result.m[0][0] = f / aspectRatio;
//    result.m[1][1] = f;
//    result.m[2][2] = farClip / (farClip - nearClip);
//    result.m[2][3] = 1.0f;
//    result.m[3][2] = -(nearClip * farClip) / (farClip - nearClip);
//    return result;
//}
//
//Matrix4x4 MakeOrthographicMatrix(float left, float top, float right,
//    float bottom, float nearClip, float farClip) {
//    Matrix4x4 m = {};
//
//    m.m[0][0] = 2.0f / (right - left);
//    m.m[1][1] = 2.0f / (top - bottom);
//    m.m[2][2] = 1.0f / (farClip - nearClip);
//    m.m[3][0] = -(right + left) / (right - left);
//    m.m[3][1] = -(top + bottom) / (top - bottom);
//    m.m[3][2] = -nearClip / (farClip - nearClip);
//    m.m[3][3] = 1.0f;
//
//    return m;
//}
//
//Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height,
//    float minDepth, float maxDepth) {
//    Matrix4x4 m = {};
//
//    m.m[0][0] = width / 2.0f;
//    m.m[1][1] = -height / 2.0f;
//    m.m[2][2] = maxDepth - minDepth;
//    m.m[3][0] = left + width / 2.0f;
//    m.m[3][1] = top + height / 2.0f;
//    m.m[3][2] = minDepth;
//    m.m[3][3] = 1.0f;
//
//    return m;
//}
//
//Vector3 Normalize(const Vector3& v) {
//    float length = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
//    if (length == 0.0f) {
//        return { 0.0f, 0.0f, 0.0f };
//    }
//    return { v.x / length, v.y / length, v.z / length };
//}

#pragma endregion

// ===============================
//  クラッシュダンプ・ログ系
// ===============================
static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	CreateDirectory(L"./Dumps", nullptr);
	StringCchPrintfW(filePath, MAX_PATH,
		L"../generated/Dumps/%04d-%02d%02d-%02d%02d.dmp",
		time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);

	HANDLE dumpFileHandle = CreateFile(
		filePath,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ,
		0, CREATE_ALWAYS, 0, 0);

	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();

	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{};
	minidumpInformation.ThreadId = threadId;
	minidumpInformation.ExceptionPointers = exception;
	minidumpInformation.ClientPointers = TRUE;

	MiniDumpWriteDump(
		GetCurrentProcess(), processId, dumpFileHandle,
		MiniDumpNormal, &minidumpInformation, nullptr, nullptr);

	return EXCEPTION_EXECUTE_HANDLER;
}

void Log(std::ostream& os, const std::string& message) {
	os << message << std::endl;
	OutputDebugStringA(message.c_str());
}

std::wstring ConvertString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	int sizeNeeded = MultiByteToWideChar(
		CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}

	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(
		CP_UTF8, 0, str.data(), static_cast<int>(str.size()),
		result.data(), sizeNeeded);
	return result;
}

std::string ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	int sizeNeeded = WideCharToMultiByte(
		CP_UTF8, 0, str.data(), static_cast<int>(str.size()),
		nullptr, 0, nullptr, nullptr);
	if (sizeNeeded == 0) {
		return std::string();
	}

	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(
		CP_UTF8, 0, str.data(), static_cast<int>(str.size()),
		result.data(), sizeNeeded, nullptr, nullptr);
	return result;
}

// ===============================
//  リソース生成系
// ===============================
ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}

ID3D12Resource* CreateTextureResource(
	ID3D12Device* device, const DirectX::TexMetadata& metadata) {

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);
	resourceDesc.Height = UINT(metadata.height);
	resourceDesc.MipLevels = UINT16(metadata.mipLevels);
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize);
	resourceDesc.Format = metadata.format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;
}

[[nodiscard]]
ID3D12Resource* UploadTextureData(
	ID3D12Resource* texture,
	const DirectX::ScratchImage& mipImages,
	ID3D12Device* device,
	ID3D12GraphicsCommandList* commandList) {

	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(
		device,
		mipImages.GetImages(),
		mipImages.GetImageCount(),
		mipImages.GetMetadata(),
		subresources);

	uint64_t intermediateSize = GetRequiredIntermediateSize(
		texture, 0, static_cast<UINT>(subresources.size()));

	ID3D12Resource* intermediate = CreateBufferResource(device, intermediateSize);

	UpdateSubresources(
		commandList, texture, intermediate,
		0, 0,
		static_cast<UINT>(subresources.size()),
		subresources.data());

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = texture;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	commandList->ResourceBarrier(1, &barrier);

	return intermediate;
}

DirectX::ScratchImage LoadTexture(const std::string& filePath) {
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(
		filePathW.c_str(),
		DirectX::WIC_FLAGS_FORCE_SRGB,
		nullptr,
		image);
	assert(SUCCEEDED(hr));

	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(
		image.GetImages(), image.GetImageCount(),
		image.GetMetadata(),
		DirectX::TEX_FILTER_SRGB,
		0, mipImages);
	assert(SUCCEEDED(hr));

	return mipImages;
}

// ===============================
//  OBJ / MTL ローダ
// ===============================
MaterialData LoadMaterialTemplateFile(
	const std::string& directoryPath,
	const std::string& filename) {

	MaterialData materialData;
	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}

	return materialData;
}

ModelData LoadObjFile(
	const std::string& directoryPath,
	const std::string& filename) {

	ModelData modelData;
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;

	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v") {
			Vector4 position{};
			s >> position.x >> position.y >> position.z;
			position.x *= -1.0f;        // 左手系に変換
			position.w = 1.0f;
			positions.push_back(position);

		} else if (identifier == "vt") {
			Vector2 texcoord{};
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);

		} else if (identifier == "vn") {
			Vector3 normal{};
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);

		} else if (identifier == "f") {
			VertexData triangle[3];

			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];

				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index);
				}

				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];

				triangle[faceVertex] = { position, texcoord, normal };
			}

			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);

		} else if (identifier == "mtllib") {
			std::string materialFilename;
			s >> materialFilename;
			modelData.material =
				LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}

	return modelData;
}

// ===============================
//  デスクリプタハンドル計算補助
// ===============================
D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(
	ID3D12DescriptorHeap* descriptorHeap,
	uint32_t descriptorSize,
	uint32_t index) {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU =
		descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(
	ID3D12DescriptorHeap* descriptorHeap,
	uint32_t descriptorSize,
	uint32_t index) {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU =
		descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}

// ===============================
//  Shader コンパイル
// ===============================
IDxcBlob* CompileShader(
	const std::wstring& filepath,
	const wchar_t* profile,
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler,
	std::ostream& os) {

	Log(os, ConvertString(
		std::format(L"Begin CompileShader, path:{}, profile:{}\n", filepath, profile)));

	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filepath.c_str(), nullptr, &shaderSource);
	assert(SUCCEEDED(hr));

	DxcBuffer shaderSourceBuffer{};
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	LPCWSTR arguments[] = {
		filepath.c_str(),
		L"-E", L"main",
		L"-T", profile,
		L"-Zi",
		L"-Qembed_debug",
		L"-Od",
		L"-Zpr",
	};

	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,
		arguments,
		_countof(arguments),
		includeHandler,
		IID_PPV_ARGS(&shaderResult));
	assert(SUCCEEDED(hr));

	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(os, shaderError->GetStringPointer());
		assert(false);
	}

	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));

	Log(os, ConvertString(
		std::format(L"Compile Succeeded, path:{}, profile:{}\n", filepath, profile)));

	shaderSource->Release();
	shaderResult->Release();

	return shaderBlob;
}

// ===============================
//  WinMain
// ===============================
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// COM 初期化
	HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));

	// 例外ダンプ設定
	SetUnhandledExceptionFilter(ExportDump);

	// ログディレクトリ
	std::filesystem::create_directory("../generated/logs");

	// ログファイル名生成（std::format を使わない版）
	auto now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);

	std::tm localTime{};
	localtime_s(&localTime, &t);   // ローカル時間に変換

	char timeStr[32]{};
	std::strftime(timeStr, sizeof(timeStr), "%Y%m%d_%H%M%S", &localTime);

	std::string dateString = timeStr;
	std::string logFilePath = "../generated/logs/" + dateString + ".log";
	std::ofstream logStream(logFilePath);

	// WinApp 初期化
	winApp = new WinApp();
	winApp->Initialize();

	// DirectXCommon 初期化
	dxCommon = new DirectXCommon();
	dxCommon->Initialize(winApp);


#pragma region スプライト共通部初期化
	// ★ スプライト共通部の初期化（資料のスライドの部分）
	spriteCommon = new SpriteCommon();    // SpriteCommon* spriteCommon = nullptr; はグローバルで定義済み
	spriteCommon->Initialize(dxCommon);
#pragma endregion

	sprite = new Sprite();
	sprite->Initialize(spriteCommon);

	// 各種ポインタ取得（DirectXCommon 経由）
	ID3D12Device* device = dxCommon->GetDevice();
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandList();
	ID3D12DescriptorHeap* srvDescriptorHeap = dxCommon->GetSRVHeap();

	// 入力
	input = new Input();
	input->Initialize(winApp);

	// DXC 初期化
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));

	// ------- ルートシグネチャ作成 -------
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1]{};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootSigDesc.pStaticSamplers = staticSamplers;
	rootSigDesc.NumStaticSamplers = _countof(staticSamplers);

	D3D12_ROOT_PARAMETER rootParameters[4]{};

	// [0] Material (CBV, Pixel)
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	// [1] Transform / WVP (CBV, Vertex)
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	// [2] Texture (DescriptorTable, SRV)
	D3D12_DESCRIPTOR_RANGE descriptorRange{};
	descriptorRange.BaseShaderRegister = 0;
	descriptorRange.NumDescriptors = 1;
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange.OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[2].ParameterType =
		D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility =
		D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

	// [3] Directional Light (CBV, Pixel)
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 1;

	rootSigDesc.pParameters = rootParameters;
	rootSigDesc.NumParameters = _countof(rootParameters);

	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(
		&rootSigDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob,
		&errorBlob);
	if (FAILED(hr)) {
		if (errorBlob) {
			Log(logStream,
				reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
		}
		assert(false);
	}

	ID3D12RootSignature* rootSignature = nullptr;
	hr = device->CreateRootSignature(
		0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	// ------- テクスチャ / モデル -------
	DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	ID3D12Resource* textureResource = CreateTextureResource(device, metadata);
	ID3D12Resource* intermediateResource =
		UploadTextureData(textureResource, mipImages, device, commandList);

	ModelData modelData = LoadObjFile("Resources", "axis.obj");

	DirectX::ScratchImage mipImages2 =
		LoadTexture(modelData.material.textureFilePath);
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	ID3D12Resource* textureResource2 = CreateTextureResource(device, metadata2);
	ID3D12Resource* intermediateResource2 =
		UploadTextureData(textureResource2, mipImages2, device, commandList);

	// ------- ディスクリプタサイズ -------
	const uint32_t descriptorSizeSRV =
		device->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// SRV ハンドル
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU =
		GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 1);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU =
		GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 1);

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 =
		GetCPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 =
		GetGPUDescriptorHandle(srvDescriptorHeap, descriptorSizeSRV, 2);

	// SRV の作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	device->CreateShaderResourceView(
		textureResource, &srvDesc, textureSrvHandleCPU);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);
	device->CreateShaderResourceView(
		textureResource2, &srvDesc2, textureSrvHandleCPU2);

	// ------- InputLayout -------
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3]{};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// ------- Blend / Rasterizer -------
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// ------- Shader コンパイル -------
	IDxcBlob* vertexShaderBlob = CompileShader(
		L"Resources/shader/Object3d.VS.hlsl",
		L"vs_6_0",
		dxcUtils, dxcCompiler, includeHandler, logStream);
	assert(vertexShaderBlob != nullptr);

	IDxcBlob* pixelShaderBlob = CompileShader(
		L"Resources/shader/Object3d.PS.hlsl",
		L"ps_6_0",
		dxcUtils, dxcCompiler, includeHandler, logStream);
	assert(pixelShaderBlob != nullptr);

	// ------- PSO -------
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.pRootSignature = rootSignature;
	psoDesc.InputLayout = inputLayoutDesc;
	psoDesc.VS = {
		vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize()
	};
	psoDesc.PS = {
		pixelShaderBlob->GetBufferPointer(),
		pixelShaderBlob->GetBufferSize()
	};
	psoDesc.BlendState = blendDesc;
	psoDesc.RasterizerState = rasterizerDesc;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.DepthStencilState = depthStencilDesc;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	psoDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	ID3D12PipelineState* graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(
		&psoDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

	// ------- モデル頂点バッファ -------
	ID3D12Resource* vertexResource =
		CreateBufferResource(
			device, sizeof(VertexData) * modelData.vertices.size());

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes =
		UINT(sizeof(VertexData) * modelData.vertices.size());
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	VertexData* vertexDataPtr = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataPtr));
	std::memcpy(
		vertexDataPtr,
		modelData.vertices.data(),
		sizeof(VertexData) * modelData.vertices.size());

	// ------- マテリアル -------
	ID3D12Resource* materialResource =
		CreateBufferResource(device, sizeof(Material));
	Material* materialData = nullptr;
	materialResource->Map(0, nullptr,
		reinterpret_cast<void**>(&materialData));
	materialData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData->enableLighting = 1;
	materialData->uvTransform = MatrixMath::MakeIdentity4x4();

	// ------- WVP -------
	ID3D12Resource* wvpResource =
		CreateBufferResource(device, sizeof(TransformationMatrix));
	TransformationMatrix* wvpData = nullptr;
	wvpResource->Map(0, nullptr,
		reinterpret_cast<void**>(&wvpData));
	Matrix4x4 identity = MatrixMath::MakeIdentity4x4();
	wvpData->WVP = identity;
	wvpData->World = identity;

	//// ------- Sprite 用 -------
	//ID3D12Resource* vertexResourceSprite =
	//    CreateBufferResource(device, sizeof(VertexData) * 4);
	//VertexData* vertexDataSprite = nullptr;
	//vertexResourceSprite->Map(0, nullptr,
	//    reinterpret_cast<void**>(&vertexDataSprite));

	//vertexDataSprite[0].position = { 0.0f, 360.0f, 0.0f, 1.0f };
	//vertexDataSprite[0].texcoord = { 0.0f, 1.0f };

	//vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };
	//vertexDataSprite[1].texcoord = { 0.0f, 0.0f };

	//vertexDataSprite[2].position = { 640.0f, 360.0f, 0.0f, 1.0f };
	//vertexDataSprite[2].texcoord = { 1.0f, 1.0f };

	//vertexDataSprite[3].position = { 640.0f, 0.0f, 0.0f, 1.0f };
	//vertexDataSprite[3].texcoord = { 1.0f, 0.0f };

	//D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	//vertexBufferViewSprite.BufferLocation =
	//    vertexResourceSprite->GetGPUVirtualAddress();
	//vertexBufferViewSprite.SizeInBytes =
	//    sizeof(VertexData) * 4;
	//vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	//ID3D12Resource* indexResourceSprite =
	//    CreateBufferResource(device, sizeof(uint32_t) * 6);
	//uint32_t* indexDataSprite = nullptr;
	//indexResourceSprite->Map(
	//    0, nullptr,
	//    reinterpret_cast<void**>(&indexDataSprite));
	//indexDataSprite[0] = 0;
	//indexDataSprite[1] = 1;
	//indexDataSprite[2] = 2;
	//indexDataSprite[3] = 1;
	//indexDataSprite[4] = 3;
	//indexDataSprite[5] = 2;

	//D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	//indexBufferViewSprite.BufferLocation =
	//    indexResourceSprite->GetGPUVirtualAddress();
	//indexBufferViewSprite.SizeInBytes =
	//    sizeof(uint32_t) * 6;
	//indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;

	//ID3D12Resource* materialResourceSprite =
	//    CreateBufferResource(device, sizeof(Material));
	//Material* materialDataSprite = nullptr;
	//materialResourceSprite->Map(
	//    0, nullptr,
	//    reinterpret_cast<void**>(&materialDataSprite));
	//materialDataSprite->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	//materialDataSprite->uvTransform = MakeIdentity4x4();
	//materialDataSprite->enableLighting = 0;

	//ID3D12Resource* transformationMatrixResourceSprite =
	//    CreateBufferResource(device, sizeof(TransformationMatrix));
	//TransformationMatrix* transformationMatrixDataSprite = nullptr;
	//transformationMatrixResourceSprite->Map(
	//    0, nullptr,
	//    reinterpret_cast<void**>(&transformationMatrixDataSprite));
	//transformationMatrixDataSprite->WVP = identity;
	//transformationMatrixDataSprite->World = identity;



	// ------- ライト -------
	ID3D12Resource* directionalLightResource =
		CreateBufferResource(device, sizeof(DirectionalLight));
	DirectionalLight* directionalLightData = nullptr;
	directionalLightResource->Map(
		0, nullptr,
		reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	directionalLightData->direction = MatrixMath::Normalize({ 0.0f, -1.0f, 0.0f });
	directionalLightData->intensity = 1.0f;

	// ------- ビューポート / シザー -------
	D3D12_VIEWPORT viewport{};
	viewport.Width = float(WinApp::kClientWidth);
	viewport.Height = float(WinApp::kClientHeight);
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.right = WinApp::kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WinApp::kClientHeight;

	// ------- 変数いろいろ -------
   /* Transform transformSprite{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};*/

	Transform transform{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};

	Transform cameraTransform{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, -5.0f}
	};

	/*  Transform uvTransformSprite{
		  {1.0f, 1.0f, 1.0f},
		  {0.0f, 0.0f, 0.0f},
		  {0.0f, 0.0f, 0.0f}
	  };*/

	bool useMonstarBall = true;

	// ------- ImGui 初期化 -------
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsClassic();

	ImGui_ImplWin32_Init(winApp->GetHwnd());
	ImGui_ImplDX12_Init(
		device,
		2,                                      // バックバッファ数
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,       // RTV フォーマット
		srvDescriptorHeap,
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	std::vector<Sprite*>sprites;
	for (uint32_t i = 0; i < 5; ++i) {
		Sprite* sprite = new Sprite();
		sprite->Initialize(spriteCommon);
		sprite->SetPosition({ float(i * 150.0f + 150.0f), 100.0f });
		sprites.push_back(sprite);
	}

	// ===============================
	//  メインループ
	// ===============================
	while (true) {

		if (winApp->ProcessMessage()) {
			break;
		}

		input->Update();

		// ImGui フレーム開始
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// --- ImGui UI ---
		ImGui::ShowDemoWindow();

		ImGui::Begin("Material / Transform");
		ImGui::SliderFloat3("Scale", &transform.scale.x, 0.1f, 5.0f);
		ImGui::SliderFloat3("RotateX", &transform.rotate.x, -180.0f, 180.0f);
		/*ImGui::SliderAngle("RotateY", &transform.rotate.y, -180.0f, 180.0f);
		ImGui::SliderAngle("RotateZ", &transform.rotate.z, -180.0f, 180.0f);*/
		ImGui::SliderFloat3("Translate", &transform.translate.x, -5.0f, 5.0f);

		ImGui::Text("useMonstarBall");
		ImGui::Checkbox("useMonstarBall", &useMonstarBall);

		ImGui::Text("Lighting Direction");
		ImGui::SliderFloat("Light X", &directionalLightData->direction.x, -10.0f, 10.0f);
		ImGui::SliderFloat("Light Y", &directionalLightData->direction.y, -10.0f, 10.0f);
		ImGui::SliderFloat("Light Z", &directionalLightData->direction.z, -10.0f, 10.0f);

		ImGui::Text("UV Transform (Sprite)");
		/*   ImGui::DragFloat2("UV Translate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		   ImGui::DragFloat2("UV Scale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		   ImGui::SliderAngle("UV Rotate", &uvTransformSprite.rotate.z);*/
		ImGui::End();

		directionalLightData->direction =
			MatrixMath::Normalize(directionalLightData->direction);

		// --- 行列計算 ---
		waveTime += 0.05f;

		Matrix4x4 worldMatrix =
			MatrixMath::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

		Matrix4x4 cameraMatrix =
			MatrixMath::MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);

		Matrix4x4 viewMatrix = MatrixMath::Inverse(cameraMatrix);

		Matrix4x4 projectionMatrix = MatrixMath::MakePerspectiveFovMatrix(
			0.45f,
			float(WinApp::kClientWidth) / float(WinApp::kClientHeight),
			0.1f, 100.0f);

		Matrix4x4 worldViewProjectionMatrix =
			MatrixMath::Multiply(worldMatrix, MatrixMath::Multiply(viewMatrix, projectionMatrix));

		wvpData->WVP = worldViewProjectionMatrix;
		wvpData->World = worldMatrix;

		/* Matrix4x4 worldMatrixSprite =
			 MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);

		 Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
		 Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(
			 0.0f, 0.0f,
			 float(WinApp::kClientWidth),
			 float(WinApp::kClientHeight),
			 0.0f, 100.0f);

		 Matrix4x4 worldViewProjectionMatrixSprite =
			 Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));

		 transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;
		 transformationMatrixDataSprite->World = worldMatrixSprite;

		 Matrix4x4 uvTransformMatrix =
			 Matrix4x4MakeScaleMatrix(uvTransformSprite.scale);
		 uvTransformMatrix = Multiply(
			 uvTransformMatrix,
			 MakeRotateZMatrix(uvTransformSprite.rotate.z));
		 uvTransformMatrix = Multiply(
			 uvTransformMatrix,
			 MakeTranslateMatrix(uvTransformSprite.translate));
		 materialDataSprite->uvTransform = uvTransformMatrix;*/

		sprite->Update();

		for (Sprite* sprite : sprites) {
			sprite->Update();

			//
			Vector2 position = sprite->GetPosition();
			//position.x += 0.1f;
			//position.y += 0.1f;
			sprite->SetPosition(position);
			//
			float rotation = sprite->GetRotation();
			rotation += 0.01f;
			sprite->SetRotation(rotation);
			//
			Vector4 color = sprite->GetColor();
			color.x += 0.01f;
			if (color.x > 1.0f) {
				color.x -= 1.0f;
			}
			sprite->SetColor(color);
			sprite->SetSize(Vector2(10.0f, 10.0f));
		}

		// ImGui コマンドを確定
		ImGui::Render();

		// =========================================
		//  DirectX 描画開始（DirectXCommon に任せる）
		// =========================================
		dxCommon->PreDraw();

		ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap };
		commandList->SetDescriptorHeaps(1, descriptorHeaps);

		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);

		commandList->SetGraphicsRootSignature(rootSignature);
		commandList->SetPipelineState(graphicsPipelineState);
		commandList->IASetPrimitiveTopology(
			D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// --- モデル描画 ---
		commandList->SetGraphicsRootConstantBufferView(
			0, materialResource->GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(
			1, wvpResource->GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(
			3, directionalLightResource->GetGPUVirtualAddress());

		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		commandList->SetGraphicsRootDescriptorTable(
			2, textureSrvHandleGPU2);    // モデルのテクスチャを使う

		commandList->DrawInstanced(
			UINT(modelData.vertices.size()), 1, 0, 0);

		//// --- スプライト描画 ---
		//commandList->IASetIndexBuffer(&indexBufferViewSprite);
		//commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);

		//commandList->SetGraphicsRootConstantBufferView(
		//    0, materialResourceSprite->GetGPUVirtualAddress());
		//commandList->SetGraphicsRootConstantBufferView(
		//    1, transformationMatrixResourceSprite->GetGPUVirtualAddress());

		//commandList->SetGraphicsRootDescriptorTable(
		//    2, textureSrvHandleGPU);   // uvChecker を使う

		//commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
		for (Sprite* sprite : sprites) {
			sprite->Draw();
		}

		spriteCommon->PreDraw();



		// --- ImGui 描画 ---
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

		// =========================================
		//  DirectX 描画終了（DirectXCommon に任せる）
		// =========================================
		dxCommon->PostDraw();
	}

	// ===============================
	//  終了処理
	// ===============================

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	graphicsPipelineState->Release();
	signatureBlob->Release();
	if (errorBlob) {
		errorBlob->Release();
	}
	rootSignature->Release();
	pixelShaderBlob->Release();
	vertexShaderBlob->Release();

	vertexResource->Release();
	materialResource->Release();
	wvpResource->Release();

	textureResource->Release();
	mipImages.Release();
	intermediateResource->Release();

	textureResource2->Release();
	mipImages2.Release();
	intermediateResource2->Release();

	/* vertexResourceSprite->Release();
	 transformationMatrixResourceSprite->Release();
	 materialResourceSprite->Release();
	 indexResourceSprite->Release();*/

	directionalLightResource->Release();

	delete input;
	input = nullptr;

	delete dxCommon;
	dxCommon = nullptr;

	winApp->Finalize();
	delete winApp;
	winApp = nullptr;

	for (Sprite* sprite : sprites) {
		delete sprite;
	}

	delete spriteCommon;
	spriteCommon = nullptr;

	delete sprite;
	sprite = nullptr;


	// DXGI Debug Live Object チェック
	IDXGIDebug1* debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}

	CoUninitialize();

	return 0;
}
