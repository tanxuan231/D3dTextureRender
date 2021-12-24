#pragma once
#include <d3d11.h>

class ShaderHelp
{
public:
	ShaderHelp();
	~ShaderHelp();

	void ErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const WCHAR* shaderFilename);
};