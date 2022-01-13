#pragma once
#include <Windows.h>
#include <d3d11.h>
#include "TextureShader.h"
#include "ColorShader.h"
#include "DxgiDuplicator.h"
#include "Nvenc.h"

#define USE_TEXTURE

class Graphics 
{
public:
	Graphics();
	~Graphics();

	void DeInit();
	bool Init(HWND hwnd, int width, int height);

	void BeginScene(float red, float green, float blue, float alpha);
	void EndScene();

	bool Render(int desktopId);

	void OnResize(int width, int height);
private:
	void DeInitColorShader();
	void DeInitDxgi();

	bool CreateDeviceAndSwapChain(HWND hwnd, int width, int height);
	bool CreateRenderTargetView();
	void SetViewports(int width, int height);
	bool InitShader();
	
private:
#ifdef  USE_TEXTURE
	TextureShader* m_shader;
#else
	ColorShader* m_shader;
#endif

	DXGIDupMgr m_dxgiDupMgr;
	NvEnc m_nvenc;
	bool m_initOver = false;

private:
	IDXGISwapChain* m_swapChain;
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;

	IDXGIDevice* m_dxgiDevice;
	IDXGIAdapter* m_dxgiAdapter;

	ID3D11RenderTargetView* m_renderTargetView;
};