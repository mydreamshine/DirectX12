//***************************************************************************************
// MathHelper.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Helper math class.
//***************************************************************************************

#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include <cstdint>

class MathHelper
{
public:
	// Returns random float in [0, 1].
	static float RandF()
	{
		return rand() / static_cast<float>(RAND_MAX);
	}

	// Returns random float in [0, 1] with mersenne_twister_engine
	static float RandF_Ex();

	// Returns random float in [a, b].
	static float RandF(float a, float b, bool Ex = false)
	{
		return a + (Ex ? RandF_Ex() : RandF())*(b - a);
	}

	// Returns random int in [a, b].
	static int Rand(int a, int b)
	{
		return a + rand() % (b - a + 1);
	}

	// Returns Minimum item between a and b.
	template<typename T>
	static T Min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	// Returns Maximum item between a and b.
	template<typename T>
	static T Max(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	// Linear Parametric
	// ex) Position by t between two vectors (a, b).
	template<typename T>
	static T Lerp(const T& a, const T& b, float t)
	{
		return a + (b - a) * t;
	}

	// Returns X with Clamp [low, high]
	template<typename T>
	static T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x);
	}

	// Returns the polar angle of the point (x,y) in [0, 2*PI).
	static float AngleFromXY(float x, float y);

	// 구면좌표(radius, theta, phi) -> 직교좌표. XM_(x,y,z,w)
	static DirectX::XMVECTOR SphericalToCartesian(float radius, float theta, float phi)
	{
		return DirectX::XMVectorSet(
			radius*sinf(phi)*cosf(theta),
			radius*cosf(phi),
			radius*sinf(phi)*sinf(theta),
			1.0f
		);
	}

	// Returns Inverse(Transpose(Matrix))
	// 모델좌표계에서 월드좌표계로 변환할 때 모델의 노멀을 정확하게 계산해 주기 위해 쓰인다.
	static DirectX::XMMATRIX InverseTranspose(DirectX::XMMATRIX M)
	{
		// Inverse-transpose is just applied to normals.  So zero out 
		// translation row so that it doesn't get into our inverse-transpose
		// calculation--we don't want the inverse-transpose of the translation.

		DirectX::XMMATRIX A = M;
		A.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(A);
		return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, A));
	}

	// Returns Identity float(4x4).
	static DirectX::XMFLOAT4X4 Identity4x4()
	{
		DirectX::XMFLOAT4X4 I(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);
		return I;
	}

	// Returns Random Unit Vector(x,y,z)
	static DirectX::XMVECTOR RandUnitVector();

	// Returns Random Unit Vector(x,y,z) in/on Hemisphere. 
	// Vector n is plane of Hemisphere.
	static DirectX::XMVECTOR RandHemisphereUnitVector(DirectX::XMVECTOR n);

	static const float Infinity;
	static const float Pi;
};

