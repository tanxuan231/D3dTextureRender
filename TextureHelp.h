#pragma once
#include <d3d11.h>
#include <string>

class TextureHelp
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
	static void SaveTex2File(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11Texture2D* texture, std::string fileName = "");

	bool CreateTextureFromFile(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11Texture2D* texture, std::string fileName);

private:
	void ClearTargaData();
	bool LoadTarga(char* filename, int& height, int& width);
	unsigned char* GetTargaData() { return m_targaData; }

private:
	unsigned char* m_targaData = nullptr;
};