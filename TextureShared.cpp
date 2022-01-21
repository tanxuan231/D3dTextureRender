#include "TextureShared.h"
#include "TextureHelp.h"
#include <d3d11_1.h>

using namespace std;

const wstring SharedTextureName(L"mySharedTextureName");

bool TextureShared::CreateTexture(ID3D11Device* device, 
	ID3D11DeviceContext* deviceContext, 
	ID3D11Texture2D* srcTex, ID3D11Texture2D** dstTex)
{
	HRESULT hr;
	D3D11_TEXTURE2D_DESC desc;
	srcTex->GetDesc(&desc);

	D3D11_TEXTURE2D_DESC sharedDesc;
	sharedDesc.Height = desc.Height;
	sharedDesc.Width = desc.Width;
	sharedDesc.Format = desc.Format;
	sharedDesc.MipLevels = desc.MipLevels;	// ²»²úÉúmip level
	sharedDesc.ArraySize = 1;
	sharedDesc.SampleDesc.Count = 1;
	sharedDesc.SampleDesc.Quality = 0;
	sharedDesc.Usage = D3D11_USAGE_DEFAULT;
	sharedDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	sharedDesc.CPUAccessFlags = 0;
	sharedDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_NTHANDLE | D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
	hr = device->CreateTexture2D(&sharedDesc, NULL, dstTex);
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

bool TextureShared::CreateTexture(ID3D11Device* device,
	ID3D11DeviceContext* deviceContext, ID3D11Texture2D** texture, string fileName)
{
	return TextureHelp::CreateTextureFromFile(device, deviceContext, texture, fileName);
}