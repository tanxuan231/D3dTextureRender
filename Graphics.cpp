#include "Graphics.h"
#include "Common.h"
#include "log.h"

#pragma comment(lib, "d3d11.lib")

Graphics::Graphics()
{
	m_shader = nullptr;

	m_swapChain = nullptr;
	m_device = nullptr;
	m_deviceContext = nullptr;
	m_renderTargetView = nullptr;
	m_dxgiDevice = nullptr;
	m_dxgiAdapter = nullptr;
}

Graphics::~Graphics()
{
	DeInit();
}

void Graphics::DeInit()
{
	m_nvenc.DeInit();

	DeInitColorShader();

	DeInitDxgi();

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

void Graphics::DeInitDxgi()
{
	if (m_dxgiAdapter)
		m_dxgiAdapter->Release();
	if (m_dxgiDevice)
		m_dxgiDevice->Release();
}

bool Graphics::Init(HWND hwnd, int width, int height)
{
	JUDGER(CreateDeviceAndSwapChain(hwnd, width, height));	
	JUDGER(CreateRenderTargetView());	
	SetViewports(width, height);
	JUDGER(InitShader());

	// DXGI抓图初始化
	m_dxgiDupMgr.EnableCursorCap();
	//m_dxgiDupMgr.EnableSave2File();
	JUDGER(m_dxgiDupMgr.Init(m_device, m_dxgiAdapter));

	// 编码开关
	m_nvenc.enable(false);
	// NV编码初始化
	m_nvenc.Init(m_device, m_deviceContext, 
		m_dxgiDupMgr.GetImageWidth(0), 
		m_dxgiDupMgr.GetImageHeight(0), 
		m_dxgiDupMgr.GetImageFormat(0),
		"out/out.h264");
	return true;
}

void Graphics::DeInitColorShader()
{
	if (m_shader) {
		m_shader->DeInit();
		delete m_shader;
		m_shader = nullptr;
	}
}

bool Graphics::InitShader()
{
#ifdef  USE_TEXTURE
	m_shader = new TextureShader();
	m_shader->SetTextureDataFile("data/stone.tga");
	return m_shader->Init(m_device, m_deviceContext, L"shader/Texture.vs", L"shader/Texture.ps");
#else
	m_shader = new ColorShader();
	return m_shader->Init(m_device, m_deviceContext, L"shader/Color.vs", L"shader/Color.ps");
#endif	
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

	// 为后台缓冲区设置常规的32位表面。
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

	// 1.创建交换链、设备及上下文
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);
	if (FAILED(hr)) {
		return false;
	}

	// 为抓取桌面图像创建DXGI设备
	// 2.创建DXGI设备
	hr = m_device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&m_dxgiDevice));
	if (FAILED(hr)) {
		return false;
	}

	// 3.获取DXGI适配器
	hr = m_dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&m_dxgiAdapter));
	if (FAILED(hr)) {
		return false;
	}

	// 3.1 枚举所有显示器
	for (int i = 0; i < 6; i++) {
		IDXGIOutput* dxgiOutput;
		HRESULT hr = m_dxgiAdapter->EnumOutputs(i, &dxgiOutput);
		if (FAILED(hr)) {			
			break;
		}
		DXGI_OUTPUT_DESC desc;
		dxgiOutput->GetDesc(&desc);
		// 输出信息
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

bool Graphics::Render(int desktopId)
{
	m_deviceContext->OMSetRenderTargets(1u, &m_renderTargetView, nullptr);

#ifdef  USE_TEXTURE
	bool result;
	ID3D11Texture2D* desktop = m_dxgiDupMgr.GetFrame(desktopId, m_device, m_deviceContext, result);
	if (!result) {
		return false;
	} else if (!desktop) {		
		return true;
	}

	// 编码
	m_nvenc.EncodeFrame(desktop);

	// 渲染
	JUDGER(m_shader->Render(m_device, m_deviceContext, desktop));
#else
	m_shader->Render(m_deviceContext);
#endif
	return true;
}