#pragma once
#include "ShaderHelp.h"
#include <Windows.h>
#include <d3d11.h>
#include <string>

class TextureShader : public ShaderHelp
{
	// 定义顶点类型（与顶点着色器中的输入类型及顶点数据布局中的数据大小保持一致）
	struct VertexType {
		VertexType(float _x, float _y, float _z,
			float _u, float _v) :
			Pos(_x, _y, _z), Tex(_u, _v) {}

		// DXGI_FORMAT_R32G32B32_FLOAT
		struct Pos {
			Pos(float _x, float _y, float _z) :
				x(_x), y(_y), z(_z) {}

			float x;
			float y;
			float z;
			// float w;	// 该分量在顶点着色器中默认设置为了1.0f
		} Pos;
		// DXGI_FORMAT_R32G32_FLOAT
		struct Tex {
			Tex(float _u, float _v) :
				u(_u), v(_v) {}

			float u;
			float v;
		} Tex;
	};

public:
	TextureShader();
	~TextureShader();

	void DeInit();
	bool Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext,
		const WCHAR* vsFilename, const WCHAR* psFilename);
	bool Render(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11Texture2D* desktopTex);

	void SetTextureDataFile(const char* fileName);
	bool UpdateTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11Texture2D* desktopTex);

private:
	void DestoryVetexInfo();
	void DestoryTextureInfo();

	bool CompileShader(const WCHAR* vsFilename, const WCHAR* psFilename);
	bool CreateShader(ID3D11Device* device);
	bool CreateInputLayout(ID3D11Device* device);
	bool CreateConstantBuffer(ID3D11Device* device);
	
	bool CreateVetexInfo(ID3D11Device* device);
	void SetInfo(ID3D11DeviceContext* deviceContext);
	bool SetInputAssemblerInfo(ID3D11DeviceContext* deviceContext);
	
	bool CreateSamplerState(ID3D11Device* device);
	void SetTextureInfo(ID3D11DeviceContext* deviceContext);
	bool CreateTextureFromFile(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	
private:
	int m_indicesCount;
	std::string m_fileName;

private:
	ID3D10Blob* m_vertexShaderBuffer;
	ID3D10Blob* m_pixelShaderBuffer;
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;

	ID3D11InputLayout* m_inputLayout;

	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;

	ID3D11SamplerState* m_sampleState;
	ID3D11Texture2D* m_texture;
	ID3D11ShaderResourceView* m_textureView;
};