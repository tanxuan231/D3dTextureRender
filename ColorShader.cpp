#include "ColorShader.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include "Common.h"
#include <cmath>

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
	m_indicesCount = 0;
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

	JUDGER(CreateTransforBuffer(device, deviceContext));

	JUDGER(CreateVetexInfo(device));
	SetInfo(deviceContext);

	return true;
}

bool ColorShader::CompileShader(const WCHAR* vsFilename, const WCHAR* psFilename)
{
	ID3D10Blob* errorMessage = nullptr;

	// 编译顶点着色器
	HRESULT hr = D3DCompileFromFile(vsFilename, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
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
	hr = D3DCompileFromFile(psFilename, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
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

bool ColorShader::CreateTransforBuffer(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	D3D11_BUFFER_DESC bufferDesc = {};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(TransBuffer);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;

	HRESULT hr = device->CreateBuffer(&bufferDesc, NULL, &m_transBuffer);
	if (FAILED(hr)) {
		return false;
	}

	deviceContext->VSSetConstantBuffers(0, 1, &m_transBuffer);

	return true;
}

bool ColorShader::UpdateTransforMatrix(ID3D11DeviceContext* deviceContext, float angle)
{
	const TransBuffer tb = {
		std::cos(angle), std::sin(angle), 0.0f, 0.0f,
		-std::sin(angle), std::cos(angle), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

	deviceContext->UpdateSubresource(m_transBuffer, 0, NULL, &tb, 0, 0);

	// 如下方式获取的pData为空
	//UINT subresource = D3D11CalcSubresource(0, 0, 0);
	//D3D11_MAPPED_SUBRESOURCE mapedRes = {};
	//HRESULT hr = deviceContext->Map(m_transBuffer, subresource, D3D11_MAP_WRITE, 0, &mapedRes);
	//if (!mapedRes.pData) {
	//	deviceContext->Unmap(m_transBuffer, subresource);
	//	return false;
	//}
	//mapedRes.pData = (void*)&tb;
	//mapedRes.RowPitch = sizeof(tb);
	//deviceContext->Unmap(m_transBuffer, subresource);

	return true;
}

bool ColorShader::CreateInputLayout(ID3D11Device* device)
{
	// 创建顶点数据布局
	const D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12u, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		// 如果用unsigned char定义颜色，比如r = 255，可以使用如下的格式，而不是DXGI_FORMAT_R8G8B8A8_UINT
		// 因为着色器需要接收浮点型，UNORM可以归一化数据
		// { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12u, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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

bool ColorShader::CreateVetexInfo(ID3D11Device* device)
{
	// 1. 顶点集合	
	VertexType vertexs[] = {
		//    顶点          |      颜色
		{-0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
		{-0.25f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
		{0.25f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
		{0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
		{0.25f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
		{-0.25f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
	};

	// 2. 顶点索引集合
	unsigned int indices[] = {
		0, 2, 1,	// 顺时针转，形成三角形
		0, 3, 2,
		0, 4, 3, 
		0, 5, 4
	};

	m_indicesCount = sizeof(indices) / sizeof(indices[0]);

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
	static int index = 0;
	UpdateTransforMatrix(deviceContext, 45.0f + index++);
	deviceContext->DrawIndexed(m_indicesCount, 0, 0);
}