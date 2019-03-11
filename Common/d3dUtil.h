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

// 아래 헤더 파일을 include하기 위해서는
// 프로젝트 속성 > VC++ 디렉터리 > 포함 디렉터리 를 수정해 줘야 한다.
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
		// 상수 버퍼는 최소 하드웨어 할당 크기(일반적으로 256 바이트)의 배수여야 합니다.
		// 따라서 256의 가장 가까운 배수로 반올림합니다.
		// 255를 더한 다음 모든 비트가 256보다 작은 하위 2 바이트를 마스크하여 이를 수행합니다.
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

// MeshGeometry에서 기하도형의 하위 범위를 정의합니다.
// 이것은 하나의 정점과 인덱스 버퍼에 여러 기하도형이 저장된 경우를 위한 것입니다.
// 그림 6.3에서 설명한 기술을 구현할 수 있도록 꼭지점 및 인덱스 버퍼에
// 기하 저장소의 하위 집합을 그리는 데 필요한 오프셋과 데이터를 제공합니다.
struct SubmeshGeometry
{
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	INT BaseVertexLocation = 0;

	// 바운딩 박스는 subMesh로 정의되어야 합니다.
	DirectX::BoundingBox Bounds;
};

struct MeshGeometry
{
	// 이름으로 해당 인스턴스를 찾을 수 있도록 이름을 지정하십시오.
	std::string Name;

	// 시스템 메모리 복사본.
	// 버텍스 / 인덱스 형식이 generic 하기 때문에 Blob을 사용하십시오.
	// 적절하게 캐스트하는 것은 클라이언트의 몫입니다.
	Microsoft::WRL::ComPtr<ID3DBlob> VertexBuferCPU = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> IndexBuferCPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuferGPU = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBuferGPU = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> VertexBuferUploader = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> IndexBuferUploader = nullptr;

	// 버퍼를 구성하는 데이터.
	UINT VertexBufferByteStride = 0; // 버퍼 원소 간격
	UINT VertexBuffserByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT; // Red 채널에 대해 16 비트를 지원하는 단일 구성 요소, 16 비트 부호없는 정수 형식입니다
	UINT IndexBufferByteSize = 0;

	// MeshGeometry는 하나의 정점/인덱스 버퍼에 여러 기하도형을 저장할 수 있습니다.
	// 이 컨테이너를 사용하여 SubmeshGeometry를 정의하면 Submesh를 개별적으로 그릴 수 있습니다.
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

	// GPU에 업로드가 끝나면 이 메모리를 확보 할 수 있습니다.
	void DisposeUploaders()
	{
		VertexBuferUploader = nullptr;
		IndexBuferUploader = nullptr;
	}
};

// 임의 조명 설정
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

// 임의 재질 설정
struct MaterialConstants
{
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f }; // Diffuse: 난반사광, Albedo: 입사광 (재질 표면에 모든 방향으로부터 입사광이 들어오고 모든방향으로 난반사가 이루어짐)
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f }; // 프레넬 렌즈 광학 계수 R0 (https://books.google.co.kr/books?id=CUfsCAAAQBAJ&pg=PA65&lpg=PA65&dq=fresnel+r0&source=bl&ots=DCICpHHRiE&sig=ACfU3U2xky1iEunsrRQ7aa-4yecZ6bOh-Q&hl=ko&sa=X&ved=2ahUKEwiUpZfg-fngAhVIyIsBHVbKBD8Q6AEwAnoECAgQAQ#v=onepage&q=fresnel%20r0&f=false)
	float Roughness = 0.25f; // Roughness: 재질의 울퉁불퉁한 정도

	// 텍스쳐 매핑에 사용.
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};

// 데모를위한 자료를 나타내는 간단한 구조체.
// 프로덕션 3D 엔진은 Materials 클래스 계층 구조를 생성합니다.
struct Material
{
	// 이름으로 인스턴스를 구분할 수 있도록 이름을 지정하십시오.
	std::string Name;

	// Index into constant buffer corresponding to this material.
	int MatCBIndex = -1;

	// Index into SRV heap for diffuse texture.
	int DiffuseSrvHeapIndex = -1;

	// Index into SRV heap for normal texture.
	int NormalSrvHeapIndex = -1;

	// 재질이 변경되었음을 나타내는 더티 플래그.
	// 상수 버퍼를 업데이트해야 합니다.
	// 각 FrameResource에 대해 재질 상수 버퍼가 있으므로
	// 각 FrameResource에 업데이트를 적용해야합니다.
	// 따라서 재질을 수정할 때 NumFramesDirty = gNumFrameResources를 설정하여
	// 각 프레임 리소스가 업데이트되도록 해야 합니다.
	int NumFramesDirty = gNumFrameResources;

	// 쉐이딩에 쓰이는 재질 상수 버퍼 데이터
	DirectX::XMFLOAT4 DiffuseAlbedo = { 1.0f, 1.0f, 1.0f, 1.0f }; // Diffuse: 난반사광, Albedo: 입사광 (재질 표면에 모든 방향으로부터 입사광이 들어오고 모든방향으로 난반사가 이루어짐)
	DirectX::XMFLOAT3 FresnelR0 = { 0.01f, 0.01f, 0.01f }; // 프레넬 렌즈 광학 계수 R0 (https://books.google.co.kr/books?id=CUfsCAAAQBAJ&pg=PA65&lpg=PA65&dq=fresnel+r0&source=bl&ots=DCICpHHRiE&sig=ACfU3U2xky1iEunsrRQ7aa-4yecZ6bOh-Q&hl=ko&sa=X&ved=2ahUKEwiUpZfg-fngAhVIyIsBHVbKBD8Q6AEwAnoECAgQAQ#v=onepage&q=fresnel%20r0&f=false)
	float Roughness = 0.25f; // Roughness: 재질의 울퉁불퉁한 정도

	// 텍스쳐 매핑에 사용.
	DirectX::XMFLOAT4X4 MatTransform = MathHelper::Identity4x4();
};

struct Texture
{
	// 이름으로 인스턴스를 구분할 수 있도록 이름을 지정하십시오.
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