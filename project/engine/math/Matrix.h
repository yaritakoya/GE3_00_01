#pragma once
#include "Struct.h"
#include "Transform.h"
struct  Matrix4x4 {
	float m[4][4];
};


struct  Matrix3x3 {
	float m[3][3];
};


namespace MatrixMath {

	//1.透視投影行列
	Matrix4x4 MakePerspectiveFovMatrix(float fovY, float aspectRatio, float nearClip, float farClip);

	//2.正射影行列
	Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip);

	//3.ビューポート変換行列
	Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth);



	//--アフィン行列--//～～～～～～～～～～～～～～～～～～～～
	//1.拡大縮小行列
	Matrix4x4 MakeScaleMatrix(const Vector3& scale);

	//2.X軸回転行列
	Matrix4x4 MakeRotateXMatrix(float radian);

	//3.Y軸回転行列
	Matrix4x4 MakeRotateYMatrix(float radian);

	//4.Z軸回転行列
	Matrix4x4 MakeRotateZMatrix(float radian);

	//5.行列の積
	Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2);

	//平行移動行列
	Matrix4x4 MakeTranslateMatrix(const Vector3& translate);

	//3次元アフィン変換行列
	Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate);
	//_________________________________________________________________________________



	//クロス積
	Vector3 Cross(const Vector3& v1, const Vector3& v2);

	//4.逆行列
	Matrix4x4 Inverse(const Matrix4x4& m);

	//3.座標変換
	Vector3 Transform(const Vector3& vector, const Matrix4x4& matrix);

	//6.単位行列の作成
	Matrix4x4 MakeIdentity4x4();

	Vector3 Normalize(const Vector3& v);
	

	
};


