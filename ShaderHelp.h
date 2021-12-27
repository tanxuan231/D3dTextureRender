#pragma once
#include <d3d11.h>

class ShaderHelp
{
	struct TargaHeader
	{
		unsigned char data1[12];
		unsigned short width;
		unsigned short height;
		unsigned char bpp;
		unsigned char data2;
	};

public:
	ShaderHelp();
	~ShaderHelp();

	void ErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const WCHAR* shaderFilename);

	bool LoadTarga(char* filename, int& height, int& width);

	unsigned char* GetTargaData() { return m_targaData; }
	void ClearTargaData();
private:
	unsigned char* m_targaData;
};