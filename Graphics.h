#pragma once
#include <Windows.h>
#include <d3d11.h>

class Graphics 
{
public:
	Graphics();
	~Graphics();

	bool Init(HWND hwnd, int width, int height);
	bool CreateDeviceAndSwapChain(HWND hwnd, int width, int height);
	bool CreateRenderTargetView();
	void SetViewports(int width, int height);

	void BeginScene(float red, float green, float blue, float alpha);
	void EndScene();

private:
	IDXGISwapChain* m_swapChain;
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;

	ID3D11RenderTargetView* m_renderTargetView;
};