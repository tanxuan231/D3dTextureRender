#include "TextureHelp.h"
#include "ShaderHelp.h"
#include "Common.h"
#include "log.h"

void TextureHelp::SaveTex2File(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ID3D11Texture2D* texture, std::string fileName)
{
    if (fileName.empty()) {
		static int index = 0;
		char fileName1[MAX_PATH] = { 0 };
		CreateDirectory(L"out/bmp", NULL);
		sprintf_s(fileName1, "out/bmp/%d.bmp", index++);
		if (index == 9) {
			index = 0;
		}
		fileName.assign(fileName1);
    }

    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);

    ID3D11Texture2D* newTexture;
    D3D11_TEXTURE2D_DESC newDesc = CD3D11_TEXTURE2D_DESC(
        desc.Format,
        desc.Width,
        desc.Height,
        1u,
        1u,
        0u,
        D3D11_USAGE_STAGING,
        D3D11_CPU_ACCESS_READ
    );

    HRESULT hr = device->CreateTexture2D(&newDesc, nullptr, &newTexture);
    if (FAILED(hr)) {
        Log(LOG_ERROR, "create texture 2d failed");
        return;
    }
    deviceContext->CopyResource(newTexture, texture);

    int destSize = desc.Height * desc.Width * 4;
    BYTE* destImage = new BYTE[destSize];

    // 8.3 纹理映射(GPU -> CPU)
    D3D11_MAPPED_SUBRESOURCE resource;
    UINT subresource = D3D11CalcSubresource(0, 0, 0);
    deviceContext->Map(newTexture, subresource, D3D11_MAP_READ, 0, &resource);
    if (!resource.pData) {
        return;
    }
    UINT rowPitch = resource.RowPitch;
    UINT size = desc.Height * resource.RowPitch;
    memcpy_s(destImage, destSize, reinterpret_cast<BYTE*>(resource.pData), size);

    deviceContext->Unmap(newTexture, subresource);

    // 保存到文件
    saveBitmap(destImage, rowPitch, desc.Height, fileName.c_str());

    delete[] destImage;

    return;
}

bool TextureHelp::CreateTextureFromFile(ID3D11Device* device, 
	ID3D11DeviceContext* deviceContext, ID3D11Texture2D** texture, std::string fileName)
{
	int height, width;
	unsigned char* data = nullptr;
	if (!LoadTarga((char*)fileName.c_str(), height, width, &data)) {
		return false;
	}
	if (!data || width <= 0 || height <= 0) {
		return false;
	}

	D3D11_TEXTURE2D_DESC textureDesc;

	textureDesc.Height = height;
	textureDesc.Width = width;
	textureDesc.MipLevels = 0;	// 完整mip level层级
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	// 1. 创建一个空的纹理
	HRESULT hr = device->CreateTexture2D(&textureDesc, NULL, texture);
	if (FAILED(hr)) {
		return false;
	}
	unsigned int rowPitch = (width * 4) * sizeof(unsigned char);

	// 2. 拷贝图像数据到纹理（加载一次即可使用UpdateSubresource）
	deviceContext->UpdateSubresource(*texture, 0, NULL, data, rowPitch, 0);

	delete[] data;

	return true;
}

bool TextureHelp::LoadTarga(char* filename, int& height, int& width, unsigned char** data)
{
	int error, bpp, imageSize, index, i, j, k;
	FILE* filePtr;
	unsigned int count;
	TargaHeader targaFileHeader;
	unsigned char* targaImage;


	// Open the targa file for reading in binary.
	error = fopen_s(&filePtr, filename, "rb");
	if (error != 0)
	{
		return false;
	}

	// Read in the file header.
	count = (unsigned int)fread(&targaFileHeader, sizeof(TargaHeader), 1, filePtr);
	if (count != 1)
	{
		return false;
	}

	// Get the important information from the header.
	height = (int)targaFileHeader.height;
	width = (int)targaFileHeader.width;
	bpp = (int)targaFileHeader.bpp;

	// Check that it is 32 bit and not 24 bit.
	if (bpp != 32)
	{
		return false;
	}

	// Calculate the size of the 32 bit image data.
	imageSize = width * height * 4;

	// Allocate memory for the targa image data.
	targaImage = new unsigned char[imageSize];
	if (!targaImage)
	{
		return false;
	}

	// Read in the targa image data.
	count = (unsigned int)fread(targaImage, 1, imageSize, filePtr);
	if (count != imageSize)
	{
		return false;
	}

	// Close the file.
	error = fclose(filePtr);
	if (error != 0)
	{
		return false;
	}

	// Allocate memory for the targa destination data.
	*data = new unsigned char[imageSize];
	if (!(*data))
	{
		return false;
	}

	// Initialize the index into the targa destination data array.
	index = 0;

	// Initialize the index into the targa image data.
	k = (width * height * 4) - (width * 4);

	// Now copy the targa image data into the targa destination array in the correct order since the targa format is stored upside down.
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
			(*data)[index + 0] = targaImage[k + 2];  // Red.
			(*data)[index + 1] = targaImage[k + 1];  // Green.
			(*data)[index + 2] = targaImage[k + 0];  // Blue
			(*data)[index + 3] = targaImage[k + 3];  // Alpha

			// Increment the indexes into the targa data.
			k += 4;
			index += 4;
		}

		// Set the targa image data index back to the preceding row at the beginning of the column since its reading it in upside down.
		k -= (width * 8);
	}

	// Release the targa image data now that it was copied into the destination array.
	delete[] targaImage;
	targaImage = 0;

	return true;
}