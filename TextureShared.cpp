#include "TextureShared.h"
#include "TextureHelp.h"
#include <d3d11_1.h>
#include <wrl\client.h>
#include "DirectXTex/DDSTextureLoader/DDSTextureLoader11.h"
#include "DirectXTex/ScreenGrab/ScreenGrab11.h"

using namespace std;
using Microsoft::WRL::ComPtr;

const wstring SharedTextureName(L"mySharedTextureName");

bool TextureShared::CreateTexture(ID3D11Device* device, 
	ID3D11DeviceContext* deviceContext, 
	ID3D11Texture2D** dstTex)
{
	ID3D11Texture2D* srcTex = nullptr;
	HRESULT hr = DirectX::CreateDDSTextureFromFile(device,
		deviceContext, 
		L"data\\test.dds",
		reinterpret_cast<ID3D11Resource**>(&srcTex),
		nullptr);
	if (FAILED(hr)) {
		return false;
	}

	DirectX::SaveDDSTextureToFile(deviceContext, srcTex, L"data\\outTex0.dds");

	D3D11_TEXTURE2D_DESC desc;
	srcTex->GetDesc(&desc);

	D3D11_TEXTURE2D_DESC dstDesc = desc;
	dstDesc.ArraySize = 1;
	dstDesc.SampleDesc.Count = 1;
	dstDesc.SampleDesc.Quality = 0;
	dstDesc.Usage = D3D11_USAGE_DEFAULT;
	//dstDesc.BindFlags = 0;// D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	dstDesc.CPUAccessFlags = 0;// D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	dstDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_NTHANDLE | D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
	hr = device->CreateTexture2D(&dstDesc, NULL, dstTex);
	if (FAILED(hr)) {
		return false;
	}

	deviceContext->CopyResource(*dstTex, srcTex);

	HANDLE handle;
	IDXGIResource1* pResource;
	(*dstTex)->QueryInterface(__uuidof(IDXGIResource1), (void**)&pResource);	
	hr = pResource->CreateSharedHandle(NULL,
		DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE,
		SharedTextureName.c_str(),
		&handle);
	pResource->Release();
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

bool TextureShared::OpenTexture(ID3D11Device* device, ID3D11Texture2D** texture)
{
	ID3D11Device1* device1;
	device->QueryInterface(__uuidof (ID3D11Device1), (void**)&device1);

	HRESULT hr = device1->OpenSharedResourceByName(SharedTextureName.c_str(),
		DXGI_SHARED_RESOURCE_READ | DXGI_SHARED_RESOURCE_WRITE,
		__uuidof(ID3D11Texture2D),
		(void**)texture);
	device1->Release();
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

bool TextureShared::SaveTexture(ID3D11DeviceContext* deviceContext, ID3D11Texture2D* texture)
{	
	D3D11_TEXTURE2D_DESC dstDesc;
	texture->GetDesc(&dstDesc);

	ComPtr<ID3D11Device> device;
	deviceContext->GetDevice(device.GetAddressOf());

	ID3D11Texture2D* tmpTex;
	dstDesc.ArraySize = 1;
	dstDesc.SampleDesc.Count = 1;
	dstDesc.SampleDesc.Quality = 0;
	dstDesc.Usage = D3D11_USAGE_DEFAULT;
	dstDesc.BindFlags = 8;
	dstDesc.CPUAccessFlags = 0;
	dstDesc.MiscFlags = 0;
	HRESULT hr = device->CreateTexture2D(&dstDesc, NULL, &tmpTex);
	if (FAILED(hr)) {
		return false;
	}

	deviceContext->CopyResource(tmpTex, texture);

	hr = DirectX::SaveDDSTextureToFile(deviceContext, tmpTex, L"data\\outTex1.dds");
	if (FAILED(hr)) {
		return false;
	}

	return true;
}