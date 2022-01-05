#pragma once

#include "nvEncodeAPI.h"
#include <d3d11.h>
#include <fstream>
#include <string>

class NvEnc
{
public:
	NvEnc();
	~NvEnc();

	void enable(bool flag);
	bool Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext,
		uint32_t width, uint32_t height, DXGI_FORMAT inputFormat, std::string outFilePath);
	void DeInit();

	bool EncodeFrame(void* frame);

private:
	bool OpenSession();
	bool InitializeEncoder();
	
	bool CreateInputBuffer(DXGI_FORMAT format);
	bool DestoryInputBuffer();

	bool RegisterResource();
	bool UnRegisterResource();

	bool CreateOutputBuffer();
	bool DestoryOutputBuffer();
private:
	bool m_enableFlag;
	std::ofstream m_outputFile;
	NV_ENCODE_API_FUNCTION_LIST m_funList;
	void* m_encoder;

	ID3D11Texture2D* m_inputTexture;
	NV_ENC_REGISTER_RESOURCE m_registerResource;
	NV_ENC_MAP_INPUT_RESOURCE m_inputMapResource;
	NV_ENC_CREATE_BITSTREAM_BUFFER m_outputBuffer;

	uint32_t m_encodeWidth = 0;
	uint32_t m_encodeHeight = 0;
	uint32_t m_frameRate = 30;
	bool m_forceNv12 = false;

	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;
};