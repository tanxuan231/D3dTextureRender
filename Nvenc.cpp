#include "Nvenc.h"
#include "exception.h"
#include <iostream>

using namespace std;

NvEnc::NvEnc()
{

}

NvEnc::~NvEnc()
{

}

bool NvEnc::Init(ID3D11Device* device, ID3D11DeviceContext* deviceContext, 
    uint32_t width, uint32_t height, std::string outFilePath)
{
    if (!device || !deviceContext || width == 0 || height == 0) {
        return false;
    }

    m_device = device;
    m_deviceContext = deviceContext;
    m_encodeWidth = width;
    m_encodeHeight = height;

    if (!outFilePath.empty()) {    
        m_outputFile.open(outFilePath, std::ios::out | std::ios::binary);
        if (m_outputFile.fail()) {
            cout << "open file: " << outFilePath << " failed" << endl;
            return false;
        }
    }

    OpenSession();
    InitializeEncoder();
    CreateInputBuffer();
    RegisterResource();
    CreateOutputBuffer();

    return true;
}

void NvEnc::DeInit()
{
    UnRegisterResource();
    DestoryInputBuffer();
    DestoryOutputBuffer();
    if (m_outputFile.is_open()) {
        m_outputFile.close();
    }
}

bool NvEnc::OpenSession()
{
    // 版本校验
    uint32_t version = 0;
    uint32_t currentVersion = (NVENCAPI_MAJOR_VERSION << 4) | NVENCAPI_MINOR_VERSION;
    NVENC_API_CALL(NvEncodeAPIGetMaxSupportedVersion(&version));
    if (currentVersion > version) {
        NVENC_THROW_ERROR("Current Driver Version does not support this NvEncodeAPI version, please upgrade driver",
            NV_ENC_ERR_INVALID_VERSION);
    }

    // 1. 加载NVENCODE API
    m_funList = { NV_ENCODE_API_FUNCTION_LIST_VER };
    NVENC_API_CALL(NvEncodeAPICreateInstance(&m_funList));
    if (!m_funList.nvEncOpenEncodeSession) {
        NVENC_THROW_ERROR("EncodeAPI not found", NV_ENC_ERR_NO_ENCODE_DEVICE);
    }

    // 2. 打开编码会话
    NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS encodeSessionExParams = { 0 };
    encodeSessionExParams.version = NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER;
    encodeSessionExParams.device = (void*)m_device;
    encodeSessionExParams.deviceType = NV_ENC_DEVICE_TYPE_DIRECTX;
    encodeSessionExParams.apiVersion = NVENCAPI_VERSION;
    NVENC_API_CALL(m_funList.nvEncOpenEncodeSessionEx(&encodeSessionExParams, &m_encoder));

    cout << "open encoder session success" << endl;

    return true;
}

bool NvEnc::InitializeEncoder()
{
    // 3 初始化编码器
    NV_ENC_PRESET_CONFIG presetConfig = { NV_ENC_PRESET_CONFIG_VER, { NV_ENC_CONFIG_VER } };
    m_funList.nvEncGetEncodePresetConfig(m_encoder, NV_ENC_CODEC_H264_GUID, NV_ENC_PRESET_P3_GUID, &presetConfig);
    NV_ENC_CONFIG config = { NV_ENC_CONFIG_VER };
    memcpy(&config, &presetConfig.presetCfg, sizeof(config));
    config.rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR;
    config.rcParams.averageBitRate = 25000;
    config.gopLength = 10;

    NV_ENC_INITIALIZE_PARAMS encoder_init_params = { NV_ENC_INITIALIZE_PARAMS_VER };
    encoder_init_params.encodeConfig = &config;
    encoder_init_params.encodeGUID = NV_ENC_CODEC_H264_GUID;
    encoder_init_params.presetGUID = NV_ENC_PRESET_P3_GUID;
    encoder_init_params.tuningInfo = NV_ENC_TUNING_INFO_ULTRA_LOW_LATENCY;
    encoder_init_params.encodeWidth = m_encodeWidth;
    encoder_init_params.encodeHeight = m_encodeHeight;
    encoder_init_params.darWidth = m_encodeWidth;
    encoder_init_params.darHeight = m_encodeHeight;
    encoder_init_params.frameRateNum = m_frameRate;  // 帧率
    encoder_init_params.frameRateDen = 1;
    encoder_init_params.enablePTD = 1;
    encoder_init_params.maxEncodeWidth = m_encodeWidth;
    encoder_init_params.maxEncodeHeight = m_encodeHeight;
    encoder_init_params.enableEncodeAsync = 0;  // 同步模式
    bool exConfig = false;
    if (exConfig) {
        NV_ENC_PRESET_CONFIG presetConfig = { NV_ENC_PRESET_CONFIG_VER, { NV_ENC_CONFIG_VER } };
        m_funList.nvEncGetEncodePresetConfigEx(m_encoder,
            NV_ENC_CODEC_H264_GUID,
            NV_ENC_PRESET_P3_GUID,
            NV_ENC_TUNING_INFO_ULTRA_LOW_LATENCY,
            &presetConfig);
        memcpy(encoder_init_params.encodeConfig, &presetConfig.presetCfg, sizeof(NV_ENC_CONFIG));
    }
    if (m_forceNv12) {
        encoder_init_params.encodeConfig->encodeCodecConfig.h264Config.chromaFormatIDC = 1; // for yuv420 input
    }
    NVENC_API_CALL(m_funList.nvEncInitializeEncoder(m_encoder, &encoder_init_params));

    return true;
}

bool NvEnc::CreateInputBuffer()
{
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
    desc.Width = m_encodeWidth;
    desc.Height = m_encodeHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    desc.CPUAccessFlags = 0;
    if (m_device->CreateTexture2D(&desc, NULL, &m_inputTexture) != S_OK) {
        NVENC_THROW_ERROR("Failed to create d3d11textures", NV_ENC_ERR_OUT_OF_MEMORY);
    }

    return true;
}

bool NvEnc::DestoryInputBuffer()
{
    if (m_inputTexture) {
        m_inputTexture->Release();
    }

    return true;
}

bool NvEnc::RegisterResource()
{
    // 4 注册输入资源    
    m_registerResource.version = NV_ENC_REGISTER_RESOURCE_VER;
    m_registerResource.resourceType = NV_ENC_INPUT_RESOURCE_TYPE_DIRECTX;
    m_registerResource.resourceToRegister = m_inputTexture;
    m_registerResource.width = m_encodeWidth;
    m_registerResource.height = m_encodeHeight;
    m_registerResource.pitch = 0;
    m_registerResource.bufferFormat = NV_ENC_BUFFER_FORMAT_NV12;
    m_registerResource.bufferUsage = NV_ENC_INPUT_IMAGE;
    m_registerResource.pInputFencePoint = NULL;
    m_registerResource.pOutputFencePoint = NULL;
    NVENC_API_CALL(m_funList.nvEncRegisterResource(m_encoder, &m_registerResource));

    // 5. 映射注册的输入资源
    m_inputMapResource.version = NV_ENC_MAP_INPUT_RESOURCE_VER;
    m_inputMapResource.registeredResource = m_registerResource.registeredResource;
    NVENC_API_CALL(m_funList.nvEncMapInputResource(m_encoder, &m_inputMapResource));

    return true;
}

bool NvEnc::UnRegisterResource()
{
    NVENC_API_CALL(m_funList.nvEncUnmapInputResource(m_encoder, m_inputMapResource.mappedResource));
    NVENC_API_CALL(m_funList.nvEncUnregisterResource(m_encoder, m_registerResource.registeredResource));

    return true;
}

bool NvEnc::CreateOutputBuffer()
{
    // 6. 创建输出比特流缓冲    
    m_outputBuffer.version = NV_ENC_CREATE_BITSTREAM_BUFFER_VER;
    NVENC_API_CALL(m_funList.nvEncCreateBitstreamBuffer(m_encoder, &m_outputBuffer));

    return true;
}

bool NvEnc::DestoryOutputBuffer()
{
    NVENC_API_CALL(m_funList.nvEncDestroyBitstreamBuffer(m_encoder, m_outputBuffer.bitstreamBuffer));

    return true;
}

bool NvEnc::EncodeFrame(void* frame)
{
    ID3D11Texture2D* frameTex = (ID3D11Texture2D*)frame;
    // 拷贝数据到输入缓冲
    //m_deviceContext->CopyResource(m_inputTexture, frameTex);
    D3D11_MAPPED_SUBRESOURCE resource;
    UINT subresource = D3D11CalcSubresource(0, 0, 0);
    m_deviceContext->Map(frameTex, subresource, D3D11_MAP_READ, 0, &resource);
    m_deviceContext->UpdateSubresource(m_inputTexture, 0, NULL, resource.pData, resource.RowPitch, 0);
    m_deviceContext->Unmap(frameTex, subresource);

    // 7 编码一帧
    NV_ENC_PIC_PARAMS picParams = { 0 };
    picParams.version = NV_ENC_PIC_PARAMS_VER;
    picParams.pictureStruct = NV_ENC_PIC_STRUCT_FRAME;
    picParams.inputBuffer = m_inputMapResource.mappedResource;
    picParams.bufferFmt = NV_ENC_BUFFER_FORMAT_NV12;
    picParams.inputWidth = m_encodeWidth;
    picParams.inputHeight = m_encodeHeight;
    picParams.outputBitstream = m_outputBuffer.bitstreamBuffer;
    picParams.inputTimeStamp = 0;
    NVENC_API_CALL(m_funList.nvEncEncodePicture(m_encoder, &picParams));

    // 8 获取输出
    NV_ENC_LOCK_BITSTREAM lockBitstreamData = { NV_ENC_LOCK_BITSTREAM_VER };
    lockBitstreamData.outputBitstream = m_outputBuffer.bitstreamBuffer;
    lockBitstreamData.doNotWait = 0;
    NVENC_API_CALL(m_funList.nvEncLockBitstream(m_encoder, &lockBitstreamData));

    unsigned char* outData = NULL;
    int dataSize = lockBitstreamData.bitstreamSizeInBytes;
    outData = new unsigned char[dataSize];
    memcpy(outData, lockBitstreamData.bitstreamBufferPtr, dataSize);

    NVENC_API_CALL(m_funList.nvEncUnlockBitstream(m_encoder, lockBitstreamData.outputBitstream));    

    if (m_outputFile.is_open()) {
        m_outputFile.write(reinterpret_cast<char*>(outData), dataSize);
        m_outputFile.flush();
    }
    delete[] outData;

    return true;
}