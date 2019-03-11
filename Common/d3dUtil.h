//***************************************************************************************
// d3dUtil.h by Frank Luna (C) 2015 All Rights Reserved.
//
// General helper code.
//***************************************************************************************

#pragma once

#include <Windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <unordered_map> // [key, value] by inserted order
#include <cstdint>
#include <fstream>
#include <sstream> // string stream
#include <cassert> // return error message in source file.

// �Ʒ� ��� ������ include�ϱ� ���ؼ���
// ������Ʈ �Ӽ� > VC++ ���͸� > ���� ���͸� �� ������ ��� �Ѵ�.
// https://www.quora.com/How-do-I-include-header-files-in-visual-C++-to-a-given-source-file
#include "d3dx12.h"
#include "DDSTextureLoader.h"
#include "MathHelper.h"

extern const int gNumFrameResources;

inline void d3dSetDebugName(IDXGIObject* obj, const char* name)
{
	if (obj)
	{
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
	}
}

inline void d3dSetDebugName(ID3D12Device* obj, const char* name)
{
	if (obj)
	{
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
	}
}

inline void d3dSetDebugName(ID3D12DeviceChild* obj, const char* name)
{
	if (obj)
	{
		obj->SetPrivateData(WKPDID_D3DDebugObjectName, lstrlenA(name), name);
	}
}

inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

class d3dUtil
{
public:

	static bool IsKeyDown(int vkeyCode);

	//static std::string ToString(HRESULT hr);

	static UINT CalcConstantBufferByteSize(UINT byteSize)
	{
		// ��� ���۴� �ּ� �ϵ���� �Ҵ� ũ��(�Ϲ������� 256 ����Ʈ)�� ������� �մϴ�.
		// ���� 256�� ���� ����� ����� �ݿø��մϴ�.
		// 255�� ���� ���� ��� ��Ʈ�� 256���� ���� ���� 2 ����Ʈ�� ����ũ�Ͽ� �̸� �����մϴ�.
		// Example: Suppose byteSize = 300.
        // (300 + 255) & ~255
        // 555 & ~255
        // 0x022B & ~0x00ff
        // 0x022B & 0xff00
        // 0x0200
        // 512
		return (byteSize + 255) & ~255;
	}

	static Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename);

	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefalutBuffer(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const void* initData,
		UINT64 byteSize,
		Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer);

	static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target);
};

class DxException
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& fileName, const int lineNumber); // return error message in source file.

	std::wstring ToString() const;
	
	HRESULT ErrorCode = S_OK;
	std::wstring FunctionName;
	std::wstring FileName;
	int LineNumber = -1;
};

// MeshGeometry���� ���ϵ����� ���� ������ �����մϴ�.
// �̰��� �ϳ��� ������ �ε��� ���ۿ� ���� ���ϵ����� ����� ��츦 ���� ���Դϴ�.
// �׸� 6.3���� ������ ����� ������ �� �ֵ��� ������ �� �ε��� ���ۿ�
// ���� ������� ���� ������ �׸��� �� �ʿ��� �����°� �����͸� �����մϴ�.
struct SubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;

	// �ٿ�� �ڽ��� subMesh�� ���ǵǾ�� �մϴ�.
	DirectX::BoundingBox Bounds;
};

struct MeshGeometry
{
	// �̸����� �ش� �ν��Ͻ��� ã�� �� �ֵ��� �̸��� �����Ͻʽÿ�.
	std::string Name;

	// �ý��� �޸� ���纻.
	// ���ؽ� / �ε��� ������ generic �ϱ� ������ Blob�� ����Ͻʽÿ�.
	// �����ϰ� ĳ��Ʈ�ϴ� ���� Ŭ���̾�Ʈ�� ���Դϴ�.
	Microsoft::WRL::ComPtr<ID3DBlob> VertexBuferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> IndexBuferCPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBuferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBuferUploader = nullptr;

	// ���۸� �����ϴ� ������.
	UINT VertexBufferByteStride = 0; // ���� ���� ����
	UINT VertexBuffserByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT; // Red ä�ο� ���� 16 ��Ʈ�� �����ϴ� ���� ���� ���, 16 ��Ʈ ��ȣ���� ���� �����Դϴ�
	UINT IndexBufferByteSize = 0;

	// MeshGeometry�� �ϳ��� ����/�ε��� ���ۿ� ���� ���ϵ����� ������ �� �ֽ��ϴ�.
	// �� �����̳ʸ� ����Ͽ� SubmeshGeometry�� �����ϸ� Submesh�� ���������� �׸� �� �ֽ��ϴ�.
	std::unordered_map<std::string, SubmeshGeometry> DrawArgs;

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView()const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBuferGPU->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertexBufferByteStride;
		vbv.SizeInBytes = VertexBuffserByteSize;

		return vbv;
	}

	D3D12_INDEX_BUFFER_VIEW IndexBufferView()const
	{
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBuferGPU->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;

		return ibv;
	}

	// GPU�� ���ε尡 ������ �� �޸𸮸� Ȯ�� �� �� �ֽ��ϴ�.
	void DisposeUploaders()
	{
		VertexBuferUploader = nullptr;
		IndexBuferUploader = nullptr;
	}
};

// ���� ���� ����
struct Light
{
	DirectX::XMFLOAT3 Strength = { 0.5f, 0.5f, 0.5f };
	float FalloffStart = 1.0f;                           // point/spot light only
	DirectX::XMFLOAT3 Direction = { 0.0f, -1.0f, 0.0f }; // directional/spot light only
	float FalloffEnd = 10.0f;                            // point/spot light only
	DirectX::XMFLOAT3 Position = { 0.0f, 0.0f, 0.0f };   // point/spot light only
	float SpotPower = 64.0f;                             // spot light only
};

#define MaxLights 16

// ���� ���� ����
struct MaterialConstants
{
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f }; // Diffuse: ���ݻ籤, Albedo: �Ի籤 (���� ǥ�鿡 ��� �������κ��� �Ի籤�� ������ ���������� ���ݻ簡 �̷����)
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f }; // ������ ���� ���� ��� R0 (https://books.google.co.kr/books?id=CUfsCAAAQBAJ&pg=PA65&lpg=PA65&dq=fresnel+r0&source=bl&ots=DCICpHHRiE&sig=ACfU3U2xky1iEunsrRQ7aa-4yecZ6bOh-Q&hl=ko&sa=X&ved=2ahUKEwiUpZfg-fngAhVIyIsBHVbKBD8Q6AEwAnoECAgQAQ#v=onepage&q=fresnel%20r0&f=false)
	float Roughness = 0.25f; // Roughness: ������ ���������� ����

	// �ؽ��� ���ο� ���.
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};

// �������� �ڷḦ ��Ÿ���� ������ ����ü.
// ���δ��� 3D ������ Materials Ŭ���� ���� ������ �����մϴ�.
struct Material
{
	// �̸����� �ν��Ͻ��� ������ �� �ֵ��� �̸��� �����Ͻʽÿ�.
	std::string Name;

	// Index into constant buffer corresponding to this material.
	int MatCBIndex = -1;

	// Index into SRV heap for diffuse texture.
	int DiffuseSrvHeapIndex = -1;

	// Index into SRV heap for normal texture.
	int NormalSrvHeapIndex = -1;

	// ������ ����Ǿ����� ��Ÿ���� ��Ƽ �÷���.
	// ��� ���۸� ������Ʈ�ؾ� �մϴ�.
	// �� FrameResource�� ���� ���� ��� ���۰� �����Ƿ�
	// �� FrameResource�� ������Ʈ�� �����ؾ��մϴ�.
	// ���� ������ ������ �� NumFramesDirty = gNumFrameResources�� �����Ͽ�
	// �� ������ ���ҽ��� ������Ʈ�ǵ��� �ؾ� �մϴ�.
	int NumFramesDirty = gNumFrameResources;

	// ���̵��� ���̴� ���� ��� ���� ������
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f }; // Diffuse: ���ݻ籤, Albedo: �Ի籤 (���� ǥ�鿡 ��� �������κ��� �Ի籤�� ������ ���������� ���ݻ簡 �̷����)
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f }; // ������ ���� ���� ��� R0 (https://books.google.co.kr/books?id=CUfsCAAAQBAJ&pg=PA65&lpg=PA65&dq=fresnel+r0&source=bl&ots=DCICpHHRiE&sig=ACfU3U2xky1iEunsrRQ7aa-4yecZ6bOh-Q&hl=ko&sa=X&ved=2ahUKEwiUpZfg-fngAhVIyIsBHVbKBD8Q6AEwAnoECAgQAQ#v=onepage&q=fresnel%20r0&f=false)
	float Roughness = 0.25f; // Roughness: ������ ���������� ����

	// �ؽ��� ���ο� ���.
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};

struct Texture
{
	// �̸����� �ν��Ͻ��� ������ �� �ֵ��� �̸��� �����Ͻʽÿ�.
	std::string Name;

	std::wstring Filename;

	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;
};

#ifndef ThrowIfFailed
#define ThrowIfFailed(x)                                              \
{                                                                     \
    HRESULT hr__ = (x);                                               \
    std::wstring wfn = AnsiToWString(__FILE__);                       \
    if(FAILED(hr__)) { throw DxException(hr__, L#x, wfn, __LINE__); } \
}
#endif

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif