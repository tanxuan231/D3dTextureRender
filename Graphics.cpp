#include "Graphics.h"

#pragma comment(lib, "d3d11.lib")

Graphics::Graphics()
{
	m_swapChain = nullptr;
	m_device = nullptr;
	m_deviceContext = nullptr;
	m_renderTargetView = nullptr;
}

Graphics::~Graphics()
{
	DeInit();
}

void Graphics::DeInit()
{
	if (m_renderTargetView) {
		m_renderTargetView->Release();
	}

	if (m_swapChain) {
		m_swapChain->Release();
	}

	if (m_deviceContext) {
		m_deviceContext->Release();
	}

	if (m_device) {
		m_device->Release();
	}
}

bool Graphics::Init(HWND hwnd, int width, int height)
{
	bool result = CreateDeviceAndSwapChain(hwnd, width, height);
	result &= CreateRenderTargetView();
	SetViewports(width, height);

	return result;
}

bool Graphics::CreateDeviceAndSwapChain(HWND hwnd, int width, int height)
{
	// 1. 创建交换链、设备和设备上下文

	DXGI_SWAP_CHAIN_DESC swapChainDesc;	
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// 设置为单个后台缓冲区。
	swapChainDesc.BufferCount = 1;

	// 设置后台缓冲区的宽度和高度。
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;

	// 为后台缓冲区设置常规的 32 位表面。
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// 设置刷新率，设置让系统尽快刷新
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

	// 设置后台缓冲区的使用目的
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// 设置要渲染到的窗口句柄
	swapChainDesc.OutputWindow = hwnd;

	// 关闭多重采样。
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// 设置为窗口模式
	swapChainDesc.Windowed = true;

	// 将扫描线排序和缩放设置为未指定。
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// 呈现后丢弃后台缓冲区内容。
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// 不要设置高级标志。
	swapChainDesc.Flags = 0;

	// 特征级别设置为 11.0，即 DirectX 11
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

bool Graphics::CreateRenderTargetView()
{
	// 2. 将后台缓冲附加到交换链上
	ID3D11Texture2D* backBufferPtr;
	HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(hr)) {
		return false;
	}

	// Create the render target view with the back buffer pointer.
	hr = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);

	// 释放资源
	backBufferPtr->Release();
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

void Graphics::SetViewports(int width, int height)
{
	// 3. 设置视窗大小
	D3D11_VIEWPORT viewport;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	m_deviceContext->RSSetViewports(1, &viewport);
}

void Graphics::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];

	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	// Clear the back buffer.
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);

	return;
}

void Graphics::EndScene()
{
	// Present as fast as possible.
	m_swapChain->Present(0, 0);	

	return;
}