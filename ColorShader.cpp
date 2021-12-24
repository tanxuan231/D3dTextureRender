#include "ColorShader.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include "Common.h"

#pragma comment(lib, "D3DCompiler.lib")

ColorShader::ColorShader()
{
	m_vertexShaderBuffer = nullptr;
	m_pixelShaderBuffer = nullptr;
	m_vertexShader = nullptr;
	m_pixelShader = nullptr;

	m_inputLayout = nullptr;
}

ColorShader::~ColorShader()
{
}

void ColorShader::DeInit()
{
	if (m_pixelShaderBuffer) {
		m_pixelShaderBuffer->Release();
	}

	if (m_vertexShaderBuffer) {
		m_vertexShaderBuffer->Release();
	}

	if (m_inputLayout) {
		m_inputLayout->Release();
	}

	if (m_pixelShader) {
		m_pixelShader->Release();
	}

	if (m_vertexShader) {
		m_vertexShader->Release();
	}
}

bool ColorShader::Init(ID3D11Device* device, const WCHAR* vsFilename, const WCHAR* psFilename)
{
	JUDGER(CompileShader(vsFilename, psFilename));
	JUDGER(CreateShader(device));
	JUDGER(CreateInputLayout(device));

	return true;
}

bool ColorShader::CompileShader(const WCHAR* vsFilename, const WCHAR* psFilename)
{
	ID3D10Blob* errorMessage = nullptr;

	// 编译顶点着色器
	HRESULT hr = D3DCompileFromFile(vsFilename, NULL, NULL, 
		"ColorVsMain",	// 着色器入口函数 
		"vs_5_0", 
		D3D10_SHADER_ENABLE_STRICTNESS, 
		0,
		&m_vertexShaderBuffer, &errorMessage);
	if (FAILED(hr)) {
		ErrorMessage(errorMessage, 0, vsFilename);
		return false;
	}

	// 编译像素着色器
	hr = D3DCompileFromFile(psFilename, NULL, NULL,
		"ColorPsMain",	// 着色器入口函数 
		"ps_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS,
		0,
		&m_vertexShaderBuffer, &errorMessage);
	if (FAILED(hr)) {
		ErrorMessage(errorMessage, 0, psFilename);
		return false;
	}

	return true;
}

bool ColorShader::CreateShader(ID3D11Device* device)
{
	// 创建着色器
	HRESULT hr = device->CreateVertexShader(
		m_vertexShaderBuffer->GetBufferPointer(),
		m_vertexShaderBuffer->GetBufferSize(),
		NULL, &m_vertexShader);
	if (FAILED(hr)) {
		return false;
	}

	hr = device->CreatePixelShader(
		m_pixelShaderBuffer->GetBufferPointer(),
		m_pixelShaderBuffer->GetBufferSize(), 
		NULL, &m_pixelShader);
	if (FAILED(hr)) {
		return false;
	}

	m_vertexShaderBuffer->Release();
	m_vertexShaderBuffer = nullptr;
	m_pixelShaderBuffer->Release();
	m_pixelShaderBuffer = nullptr;

	return true;
}

bool ColorShader::CreateInputLayout(ID3D11Device* device)
{
	// 创建顶点数据布局
	const D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8u, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HRESULT hr = device->CreateInputLayout(
		inputElementDescs, 
		sizeof(inputElementDescs)/sizeof(D3D11_INPUT_ELEMENT_DESC),
		m_vertexShaderBuffer->GetBufferPointer(),
		m_vertexShaderBuffer->GetBufferSize(), 
		&m_inputLayout);
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

bool ColorShader::CreateConstantBuffer(ID3D11Device* device)
{
	// 创建常量缓冲，用于跟着色器交换数据。当前不需要
	ID3D11Buffer* buffer;

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(UINT);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	HRESULT hr = device->CreateBuffer(&bufferDesc, NULL, &buffer);
	if (FAILED(hr)) {
		return false;
	}

	return true;
}