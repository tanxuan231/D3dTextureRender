#pragma once
#include "ShaderHelp.h"
#include <Windows.h>
#include <d3d11.h>

class TextureShader : public ShaderHelp
{
	// 定义顶点类型（与顶点着色器中的输入类型及顶点数据布局中的数据大小保持一致）
	struct VertexType {
		VertexType(float _x, float _y, float _z,
			float _r, float _g, float _b, float _a) :
			Pos(_x, _y, _z), Color(_r, _g, _b, _a) {}

		// DXGI_FORMAT_R32G32B32_FLOAT
		struct Pos {
			Pos(float _x, float _y, float _z) :
				x(_x), y(_y), z(_z) {}

			float x;
			float y;
			float z;
			// float w;	// 该分量在顶点着色器中默认设置为了1.0f
		} Pos;
		// DXGI_FORMAT_R32G32B32A32_FLOAT
		struct Color {
			Color(float _r, float _g, float _b, float _a) :
				r(_r), g(_g), b(_b), a(_a) {}

			float r;
			float g;
			float b;
			float a;
		} Color;
	};

public:
	TextureShader();
	~TextureShader();

	void DeInit();
	bool Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext,
		const WCHAR* vsFilename, const WCHAR* psFilename);
	void Render(ID3D11DeviceContext* deviceContext);

private:
	void DestoryVetexInfo();

	bool CompileShader(const WCHAR* vsFilename, const WCHAR* psFilename);
	bool CreateShader(ID3D11Device* device);
	bool CreateInputLayout(ID3D11Device* device);
	bool CreateConstantBuffer(ID3D11Device* device);
	
	bool CreateVetexInfo(ID3D11Device* device);
	void SetInfo(ID3D11DeviceContext* deviceContext);
	bool SetInputAssemblerInfo(ID3D11DeviceContext* deviceContext);

private:
	int m_indicesCount;

private:
	ID3D10Blob* m_vertexShaderBuffer;
	ID3D10Blob* m_pixelShaderBuffer;
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;

	ID3D11InputLayout* m_inputLayout;

	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;
};