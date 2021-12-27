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

	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_vertexCount = 0;
}

ColorShader::~ColorShader()
{
}

void ColorShader::DeInit()
{
	DestoryVetexInfo();

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

void ColorShader::DestoryVetexInfo()
{
	if (m_vertexBuffer) {
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}

	if (m_indexBuffer) {
		m_indexBuffer->Release();
		m_indexBuffer = nullptr;
	}
}

bool ColorShader::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, 
	const WCHAR* vsFilename, const WCHAR* psFilename)
{
	JUDGER(CompileShader(vsFilename, psFilename));
	JUDGER(CreateShader(device));
	JUDGER(CreateInputLayout(device));

	JUDGER(CreateVetexInfo(device));
	SetInfo(deviceContext);

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
		&m_pixelShaderBuffer, &errorMessage);
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

	return true;
}

bool ColorShader::CreateInputLayout(ID3D11Device* device)
{
	// 创建顶点数据布局
	const D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12u, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

	m_vertexShaderBuffer->Release();
	m_vertexShaderBuffer = nullptr;
	m_pixelShaderBuffer->Release();
	m_pixelShaderBuffer = nullptr;

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

bool ColorShader::CreateVetexInfo(ID3D11Device* device)
{
	// 1. 顶点集合	
	VertexType vertexs[] = {
		{-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
		{0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
		{1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f}
	};
	m_vertexCount = sizeof(vertexs) / sizeof(vertexs[0]);

	// 2. 顶点索引集合
	unsigned int indices[] = {
		0, 1, 2
	};

	// 3. 创建顶点缓冲
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(vertexs);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = vertexs;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(hr)) {
		return false;
	}

	// 4. 创建顶点索引缓冲
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA indexData;

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(indices);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	hr = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

bool ColorShader::SetInputAssemblerInfo(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// 绑定顶点缓存到渲染管线的输入装配阶段（input assembler）
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// 绑定索引缓存到渲染管线的输入装配阶段（input assembler）
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 设置图元拓扑
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_inputLayout);

	return true;
}

void ColorShader::SetInfo(ID3D11DeviceContext* deviceContext)
{
	SetInputAssemblerInfo(deviceContext);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);	
}

void ColorShader::Render(ID3D11DeviceContext* deviceContext)
{
	// Render the triangle.
	deviceContext->DrawIndexed(m_vertexCount, 0, 0);
}