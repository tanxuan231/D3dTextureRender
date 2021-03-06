#pragma once
#include <d3d11.h>
#include <string>

class TextureShared
{
public:
	bool CreateTexture(ID3D11Device* device,
		ID3D11DeviceContext* deviceContext,
		ID3D11Texture2D** dstTex);

	bool OpenTexture(ID3D11Device* device, ID3D11Texture2D** texture);

	bool SaveTexture(ID3D11DeviceContext* deviceContext, ID3D11Texture2D* texture);
};