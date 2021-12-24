#pragma once
#include "ShaderHelp.h"
#include <Windows.h>
#include <d3d11.h>

class ColorShader : public ShaderHelp
{
public:
	ColorShader();
	~ColorShader();

	void DeInit();
	bool Init(ID3D11Device* device, const WCHAR* vsFilename, const WCHAR* psFilename);

private:
	bool CompileShader(const WCHAR* vsFilename, const WCHAR* psFilename);
	bool CreateShader(ID3D11Device* device);
	bool CreateInputLayout(ID3D11Device* device);
	bool CreateConstantBuffer(ID3D11Device* device);

private:
	ID3D10Blob* m_vertexShaderBuffer;
	ID3D10Blob* m_pixelShaderBuffer;
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;

	ID3D11InputLayout* m_inputLayout;

};