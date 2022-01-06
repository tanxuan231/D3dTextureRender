#include "TextureShader.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include "Common.h"
#include "log.h"

#pragma comment(lib, "D3DCompiler.lib")

TextureShader::TextureShader()
{
	m_vertexShaderBuffer = nullptr;
	m_pixelShaderBuffer = nullptr;
	m_vertexShader = nullptr;
	m_pixelShader = nullptr;

	m_inputLayout = nullptr;

	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_indicesCount = 0;

	m_sampleState = nullptr;
	m_texture = nullptr;
	m_textureView = nullptr;
}

TextureShader::~TextureShader()
{
}

void TextureShader::DeInit()
{
	DestoryVetexInfo();
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

void TextureShader::DestoryVetexInfo()
{
	if (m_sampleState) {
		m_sampleState->Release();
		m_sampleState = nullptr;
	}

	if (m_textureView) {
		m_textureView->Release();
		m_textureView = nullptr;
	}

	if (m_texture) {
		m_texture->Release();
		m_texture = nullptr;
	}
}

void TextureShader::DestoryTextureInfo()
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

void TextureShader::SetTextureDataFile(const char* fileName)
{	
	m_fileName.assign(fileName);	
}

bool TextureShader::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, 
	const WCHAR* vsFilename, const WCHAR* psFilename)
{
	JUDGER(CompileShader(vsFilename, psFilename));
	JUDGER(CreateShader(device));
	JUDGER(CreateInputLayout(device));
	JUDGER(CreateSamplerState(device));
	//JUDGER(CreateTextureFromFile(device, deviceContext));

	JUDGER(CreateVetexInfo(device));
	SetInfo(deviceContext);

	return true;
}

bool TextureShader::CompileShader(const WCHAR* vsFilename, const WCHAR* psFilename)
{
	ID3D10Blob* errorMessage = nullptr;

	// ���붥����ɫ��
	HRESULT hr = D3DCompileFromFile(vsFilename, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"TextureVsMain",	// ��ɫ����ں��� 
		"vs_5_0", 
		D3D10_SHADER_ENABLE_STRICTNESS, 
		0,
		&m_vertexShaderBuffer, &errorMessage);
	if (FAILED(hr)) {
		ErrorMessage(errorMessage, 0, vsFilename);
		return false;
	}

	// ����������ɫ��
	hr = D3DCompileFromFile(psFilename, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"TexturePsMain",	// ��ɫ����ں��� 
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

bool TextureShader::CreateShader(ID3D11Device* device)
{
	// ������ɫ��
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

bool TextureShader::CreateConstantBuffer(ID3D11Device* device)
{
	// �����������壬���ڸ���ɫ���������ݡ���ǰ����Ҫ
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

bool TextureShader::CreateInputLayout(ID3D11Device* device)
{
	// �����������ݲ���
	const D3D11_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },	// float4����
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12u, D3D11_INPUT_PER_VERTEX_DATA, 0 },	// float2����
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

bool TextureShader::CreateSamplerState(ID3D11Device* device)
{
	// �����������״̬
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	HRESULT hr = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(hr)) {
		return false;
	}

	return true;
}

bool TextureShader::CreateVetexInfo(ID3D11Device* device)
{
	// 1. ���㼯��	
	VertexType vertexs[] = {
		{-1.0f, -1.0f, 0.0f, 0.0f, 1.0f},	// ���½�
		{1.0f, 1.0f, 0.0f, 1.0f, 0.0f},		// ���Ͻ�
		{1.0f, -1.0f, 0.0f, 1.0f, 1.0f},	// ���½�
		{-1.0f, 1.0f, 0.0f, 0.0f, 0.0f}		// ���Ͻ�
	};	

	// 2. ������������
	unsigned int indices[] = {
		0, 1, 2,	// ��һ�������Σ����¶�����˳ʱ�룩
		0, 3, 1		// �ڶ���������
	};

	m_indicesCount = sizeof(indices) / sizeof(indices[0]);

	// 3. �������㻺��
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

	// 4. ����������������
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

bool TextureShader::CreateTextureFromFile(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	int height, width;
	if (!LoadTarga((char*)m_fileName.c_str(), height, width)) {
		return false;
	}

	D3D11_TEXTURE2D_DESC textureDesc;

	textureDesc.Height = height;
	textureDesc.Width = width;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	// 1. ����һ���յ�����
	HRESULT hr = device->CreateTexture2D(&textureDesc, NULL, &m_texture);
	if (FAILED(hr)) {
		return false;
	}
	
	unsigned int rowPitch = (width * 4) * sizeof(unsigned char);

	// 2. ����ͼ�����ݵ���������һ�μ���ʹ��UpdateSubresource��
	deviceContext->UpdateSubresource(m_texture, 0, NULL, GetTargaData(), rowPitch, 0);
	
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;	// Mipmap����

	// 3. Ϊ��������ɫ����Դ��ͼ
	hr = device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView);
	if (FAILED(hr)) {
		return false;
	}

	// 4. Ϊ������Դ��ͼ����Mipmap�㼶
	deviceContext->GenerateMips(m_textureView);

	ClearTargaData();

	return true;
}

bool TextureShader::UpdateTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11Texture2D* desktopTex)
{
	if (!desktopTex) {
		return true;
	}

	D3D11_TEXTURE2D_DESC desc;
	desktopTex->GetDesc(&desc);

	// 1. ����һ���յ�����
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Height = desc.Height;
	textureDesc.Width = desc.Width;
	textureDesc.Format = desc.Format;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;	
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;	
	HRESULT hr = device->CreateTexture2D(&textureDesc, NULL, &m_texture);
	if (FAILED(hr)) {
		return false;
	}

	// 2. ����ͼ�����ݵ���������һ�μ���ʹ��UpdateSubresource��
#if 1
	D3D11_MAPPED_SUBRESOURCE resource;
	UINT subresource = D3D11CalcSubresource(0, 0, 0);
	deviceContext->Map(desktopTex, subresource, D3D11_MAP_READ, 0, &resource);
	deviceContext->UpdateSubresource(m_texture, 0, NULL, resource.pData, resource.RowPitch, 0);
	deviceContext->Unmap(desktopTex, subresource);
#else
	deviceContext->CopyResource(m_texture, desktopTex);
#endif

	// 3. Ϊ��������ɫ����Դ��ͼ(ע�⣺����BindFlags��Ҫ����ΪD3D11_BIND_SHADER_RESOURCE)
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;	// Mipmap����
	hr = device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView);
	m_texture->Release();
	if (FAILED(hr)) {
		return false;
	}

	// 4. Ϊ������Դ��ͼ����Mipmap�㼶
	deviceContext->GenerateMips(m_textureView);
	
	// ��������Դ��ͼ��������ɫ��
	deviceContext->PSSetShaderResources(0, 1, &m_textureView);
	return true;
}

bool TextureShader::SetInputAssemblerInfo(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// �󶨶��㻺�浽��Ⱦ���ߵ�����װ��׶Σ�input assembler��
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// ���������浽��Ⱦ���ߵ�����װ��׶Σ�input assembler��
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// ����ͼԪ����
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set the vertex input layout.
	deviceContext->IASetInputLayout(m_inputLayout);

	return true;
}

void TextureShader::SetTextureInfo(ID3D11DeviceContext* deviceContext)
{
	// ��������Դ��ͼ��������ɫ��
	//deviceContext->PSSetShaderResources(0, 1, &m_textureView);

	// �󶨲���״̬��������ɫ��
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);
}

void TextureShader::SetInfo(ID3D11DeviceContext* deviceContext)
{
	SetInputAssemblerInfo(deviceContext);

	SetTextureInfo(deviceContext);

	// Set the vertex and pixel shaders that will be used to render this triangle.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);	
}

bool TextureShader::Render(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11Texture2D* desktopTex)
{
	if (!UpdateTexture(device, deviceContext, desktopTex)) {
		Log(LOG_ERROR, "UpdateTexture failed");
		return false;
	}
	
	deviceContext->DrawIndexed(m_indicesCount, 0, 0);

	return true;
}