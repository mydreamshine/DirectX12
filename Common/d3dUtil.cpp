#include "d3dUtil.h"
#include <comdef.h>
#include <fstream>

using Microsoft::WRL::ComPtr;

DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::wstring & fileName, const int lineNumber) :
	ErrorCode(hr),
	FunctionName(functionName),
	FileName(fileName),
	LineNumber(lineNumber)
{
}

bool d3dUtil::IsKeyDown(int vkeyCode)
{
	return (GetAsyncKeyState(vkeyCode) & 0x8000) != 0; // & 0x8000 : key down (is not pressed)
}

Microsoft::WRL::ComPtr<ID3DBlob> d3dUtil::LoadBinary(const std::wstring & filename)
{
	std::fstream fin(filename, std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	std::ifstream::pos_type size = (int)fin.tellg(); // bytesize of file
	fin.seekg(0, std::ios_base::beg);

	ComPtr<ID3DBlob> blob;
	ThrowIfFailed(D3DCreateBlob(size, blob.GetAddressOf()));

	fin.read((char*)blob->GetBufferPointer(), size);
	fin.close();

	return blob;
}

Microsoft::WRL::ComPtr<ID3D12Resource> d3dUtil::CreateDefalutBuffer(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	const void* initData,
	UINT64 byteSize,
	Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
{
	ComPtr<ID3D12Resource> defalutBffer;

	// 실제 기본 버퍼 리소스를 만듭니다.
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defalutBffer.GetAddressOf())));

	// CPU 메모리 데이터를 기본 버퍼에 복사하려면 중간 업로드 힙을 만들어야합니다.
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())));

	// 기본 버퍼에 복사 할 데이터를 설명하십시오.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// 데이터를 기본 버퍼 리소스에 복사하도록 예약합니다.
	// 높은 수준에서 Helper 함수 UpdateSubresources는
	// CPU 메모리를 중간 업로드 힙으로 복사합니다.
	// 그런 다음 ID3D12CommandList::CopySubresourceRegion을 사용하여 중간 업로드 힙 데이터가 Buffer에 복사됩니다.
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defalutBffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	UpdateSubresources<1>(cmdList, defalutBffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defalutBffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	// 참고:
	// 실제 복사를 수행하는 명령 목록이 아직 실행되지 않았기 때문에
	// uploadBuffer는 위의 함수 호출 후에도 유지되어야합니다.
	// caller는 복사가 실행 된 것을 알고 나서, uploadBuffer를 릴리즈 할 수 있습니다.

	return defalutBffer;
}

Microsoft::WRL::ComPtr<ID3DBlob> d3dUtil::CompileShader(
	const std::wstring& filename,
	const D3D_SHADER_MACRO* defines,
	const std::string& entrypoint,
	const std::string& target)
{
	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = S_OK;

	ComPtr<ID3DBlob> byteCode = nullptr;
	ComPtr<ID3DBlob> errors;
	hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

	if (errors != nullptr)
		OutputDebugStringA((char*)errors->GetBufferPointer());

	ThrowIfFailed(hr);

	return byteCode;
}

std::wstring DxException::ToString() const
{
	// error code에 대한 설명구를 얻습니다.
	_com_error err(ErrorCode);
	std::wstring msg = AnsiToWString(err.ErrorMessage());

	return FunctionName + L" failed in " + FileName + L"; line " + std::to_wstring(LineNumber) + L"; error: " + msg;
}

