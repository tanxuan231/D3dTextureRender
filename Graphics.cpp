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
	// 1. �������������豸���豸������

	DXGI_SWAP_CHAIN_DESC swapChainDesc;	
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	// ����Ϊ������̨��������
	swapChainDesc.BufferCount = 1;

	// ���ú�̨�������Ŀ�Ⱥ͸߶ȡ�
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;

	// Ϊ��̨���������ó���� 32 λ���档
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// ����ˢ���ʣ�������ϵͳ����ˢ��
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

	// ���ú�̨��������ʹ��Ŀ��
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// ����Ҫ��Ⱦ���Ĵ��ھ��
	swapChainDesc.OutputWindow = hwnd;

	// �رն��ز�����
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	// ����Ϊ����ģʽ
	swapChainDesc.Windowed = true;

	// ��ɨ�����������������Ϊδָ����
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// ���ֺ�����̨���������ݡ�
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// ��Ҫ���ø߼���־��
	swapChainDesc.Flags = 0;

	// ������������Ϊ 11.0���� DirectX 11
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
	// 2. ����̨���帽�ӵ���������
	ID3D11Texture2D* backBufferPtr;
	HRESULT hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(hr)) {
		return false;
	}

	// Create the render target view with the back buffer pointer.
	hr = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);

	// �ͷ���Դ
	backBufferPtr->Release();
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

void Graphics::SetViewports(int width, int height)
{
	// 3. �����Ӵ���С
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