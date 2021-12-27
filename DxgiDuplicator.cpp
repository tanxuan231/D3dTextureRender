// DxgiDuplicator.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "DxgiDuplicator.h"
#include "Common.h"
#include <iostream>

#pragma comment(lib, "DXGI.lib")

using namespace std;

DXGIDupMgr::DXGIDupMgr()
{}

DXGIDupMgr::~DXGIDupMgr()
{
    for (auto e : m_texture2dV) {
        e->Release();
    }
    for (auto e : m_outputDupV) {
        e->Release();
    }
    for (auto e : m_dxgiOutput1V) {
        e->Release();
    }
    for (auto e : m_dxgiOutputV) {
        e->Release();
    }
}

bool DXGIDupMgr::Init(ID3D11Device* device, IDXGIAdapter* dxgiAdapter)
{
    JUDGER(InitOutput(0, device, dxgiAdapter));
    JUDGER(InitOutput(1, device, dxgiAdapter));

    return true;
}

bool DXGIDupMgr::InitOutput(int monitorIdx, ID3D11Device* device, IDXGIAdapter* dxgiAdapter)
{
    // 4.获取DXGI output
    IDXGIOutput* dxgiOutput;
    HRESULT hr = dxgiAdapter->EnumOutputs(monitorIdx, &dxgiOutput);
    if (FAILED(hr)) {
        cout << "failed for get EnumOutputs" << endl;
        return false;
    }     
    m_dxgiOutputV.emplace_back(dxgiOutput);

    // 5.获取DXGI output1  
    IDXGIOutput1* dxgiOutput1;
    hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&dxgiOutput1));
    if (FAILED(hr)) {
        cout << "failed for get IDXGIOutput1" << endl;
        return false;
    }
    m_dxgiOutput1V.emplace_back(dxgiOutput1);

    // 6.获取DXGI OutputDuplication    
    IDXGIOutputDuplication* outputDup;
    hr = dxgiOutput1->DuplicateOutput(device, &outputDup);
    if (FAILED(hr)) {
        cout << "failed for DuplicateOutput" << endl;
        if (hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE) {
            cout << "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE" << endl;
        }
        return false;
    }
    m_outputDupV.emplace_back(outputDup);

    // 创建纹理
    DXGI_OUTDUPL_DESC dxgiOutduplDesc;
    outputDup->GetDesc(&dxgiOutduplDesc);
    if (!CreateTexture(device, dxgiOutduplDesc.ModeDesc.Width,
        dxgiOutduplDesc.ModeDesc.Height,
        dxgiOutduplDesc.ModeDesc.Format)) {
        return false;
    }

    return true;
}

bool DXGIDupMgr::CreateTexture(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format)
{
    ID3D11Texture2D* texture2d;
    D3D11_TEXTURE2D_DESC texture2dDesc;

    texture2dDesc.Width = width;
    texture2dDesc.Height = height;
    texture2dDesc.Format = format;
    texture2dDesc.ArraySize = 1;
    texture2dDesc.BindFlags = 0;
    texture2dDesc.MiscFlags = 0;
    texture2dDesc.SampleDesc.Count = 1;
    texture2dDesc.SampleDesc.Quality = 0;
    texture2dDesc.MipLevels = 1;
    texture2dDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    texture2dDesc.Usage = D3D11_USAGE_STAGING;

    HRESULT hr = device->CreateTexture2D(&texture2dDesc, NULL, &texture2d);
    if (FAILED(hr)) {
        return false;
    }
    m_texture2dV.emplace_back(texture2d);

    return true;
}

ID3D11Texture2D* DXGIDupMgr::GetFrame(int idx, ID3D11DeviceContext* deviceContext, void* destImage, UINT destSize, UINT* rowPitch)
{
    cout << "get frame start" << endl;
    *rowPitch = 0;

    // 7. 获取桌面图像
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    IDXGIResource* idxgiRes;
    HRESULT hr = m_outputDupV[idx]->AcquireNextFrame(500, &frameInfo, &idxgiRes);
    if (FAILED(hr)) {
        printf("failed for AcquireNextFrame: %x\n", hr);
        if (hr == DXGI_ERROR_ACCESS_LOST)
            cout << "DXGI_ERROR_ACCESS_LOST" << endl;
        if (hr == DXGI_ERROR_INVALID_CALL)
            cout << "DXGI_ERROR_INVALID_CALL " << endl;
        if (hr == E_INVALIDARG)
            cout << "E_INVALIDARG  " << endl;
        if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
            cout << "DXGI_ERROR_WAIT_TIMEOUT  " << endl;
            return nullptr;
        }
        return nullptr;
    }

    ID3D11Texture2D* desktopTexture2d;
    hr = idxgiRes->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&desktopTexture2d));
    if (FAILED(hr)) {
        cout << "failed for get desktopTexture2d" << endl;
        return nullptr;
    }

    // 【资源释放】9.1 查询到数据后，释放IDXG资源
    idxgiRes->Release();
    // 8. 将桌面图像拷贝出来
    // 8.2 复制纹理(GPU间复制)
    deviceContext->CopyResource(m_texture2dV[idx], desktopTexture2d);

    // 【资源释放】9.2 拷贝完数据后，释放桌面纹理
    desktopTexture2d->Release();

    // 【资源释放】9.3 需要释放帧，对应AcquireNextFrame
    m_outputDupV[idx]->ReleaseFrame();

    return m_texture2dV[idx];
}