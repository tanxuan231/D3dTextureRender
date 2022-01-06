// DxgiDuplicator.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "DxgiDuplicator.h"
#include "Common.h"
#include "log.h"

#pragma comment(lib, "DXGI.lib")

using namespace std;

DXGIDupMgr::DXGIDupMgr()
{
    m_cursorTex = nullptr;

    m_save2file = false;
    m_cursorCap = false;
}

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
    if (m_cursorTex) {
        m_cursorTex->Release();
    }
}

bool DXGIDupMgr::Init(ID3D11Device* device, IDXGIAdapter* dxgiAdapter)
{
    JUDGER(InitOutput(0, device, dxgiAdapter));
    //JUDGER(InitOutput(1, device, dxgiAdapter));

    return true;
}

bool DXGIDupMgr::InitOutput(int monitorIdx, ID3D11Device* device, IDXGIAdapter* dxgiAdapter)
{
    // 4.获取DXGI output
    IDXGIOutput* dxgiOutput;
    HRESULT hr = dxgiAdapter->EnumOutputs(monitorIdx, &dxgiOutput);
    if (FAILED(hr)) {
        Log(LOG_ERROR, "failed for get EnumOutputs");
        return false;
    }     
    m_dxgiOutputV.emplace_back(dxgiOutput);

    // 5.获取DXGI output1  
    IDXGIOutput1* dxgiOutput1;
    hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&dxgiOutput1));
    if (FAILED(hr)) {
        Log(LOG_ERROR, "failed for get IDXGIOutput1");
        return false;
    }
    m_dxgiOutput1V.emplace_back(dxgiOutput1);

    // 6.获取DXGI OutputDuplication    
    IDXGIOutputDuplication* outputDup;
    hr = dxgiOutput1->DuplicateOutput(device, &outputDup);
    if (FAILED(hr)) {
        Log(LOG_ERROR, "failed for DuplicateOutput");
        if (hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE) {
            Log(LOG_ERROR, "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE");
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
    texture2dDesc.SampleDesc.Count = 1;
    texture2dDesc.SampleDesc.Quality = 0;    
    texture2dDesc.MipLevels = 1;
    texture2dDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    texture2dDesc.BindFlags = 0;
    texture2dDesc.MiscFlags = 0;
    texture2dDesc.Usage = D3D11_USAGE_STAGING;  // 中转纹理

    HRESULT hr = device->CreateTexture2D(&texture2dDesc, NULL, &texture2d);
    if (FAILED(hr)) {
        return false;
    }
    m_texture2dV.emplace_back(texture2d);

    // 当设置为D3D11_RESOURCE_MISC_GDI_COMPATIBLE后，
    // BindFlags必须为D3D11_BIND_RENDER_TARGET 
    // Usage必须具备GPU读写即D3D11_USAGE_DEFAULT
    // 不能有多重采样，格式有要求
    texture2dDesc.MiscFlags = D3D11_RESOURCE_MISC_GDI_COMPATIBLE;
    texture2dDesc.CPUAccessFlags = 0;
    texture2dDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
    texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
    texture2dDesc.SampleDesc.Count = 1;
    texture2dDesc.SampleDesc.Quality = 0;    
    texture2dDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;

    hr = device->CreateTexture2D(&texture2dDesc, NULL, &m_cursorTex);
    if (FAILED(hr)) {
        return false;
    }

    return true;
}

ID3D11Texture2D* DXGIDupMgr::GetFrame(int idx, ID3D11DeviceContext* deviceContext, bool& result)
{
    result = false;
    // 7. 获取桌面图像
    DXGI_OUTDUPL_FRAME_INFO frameInfo;
    IDXGIResource* idxgiRes;
    HRESULT hr = m_outputDupV[idx]->AcquireNextFrame(1000, &frameInfo, &idxgiRes);
    if (FAILED(hr)) {
        Log(LOG_ERROR, "failed for AcquireNextFrame: %x", hr);
        if (hr == DXGI_ERROR_ACCESS_LOST)
            Log(LOG_ERROR, "DXGI_ERROR_ACCESS_LOST");
        if (hr == DXGI_ERROR_INVALID_CALL)
            Log(LOG_ERROR, "DXGI_ERROR_INVALID_CALL");
        if (hr == E_INVALIDARG)
            Log(LOG_ERROR, "E_INVALIDARG");
        if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
            Log(LOG_ERROR, "DXGI_ERROR_WAIT_TIMEOUT");
            result = true;
            return nullptr;
        }        
        return nullptr;
    }

    ID3D11Texture2D* desktopTexture2d;
    hr = idxgiRes->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&desktopTexture2d));
    // 【资源释放】9.1 查询到数据后，释放IDXG资源
    idxgiRes->Release();
    if (FAILED(hr)) {
        Log(LOG_ERROR, "failed for get desktopTexture2d");
        return nullptr;
    }

    D3D11_TEXTURE2D_DESC desktopDesc;
    desktopTexture2d->GetDesc(&desktopDesc);

    // 8. 将桌面图像拷贝出来
    // 8.2 复制纹理(GPU间复制)    
    deviceContext->CopyResource(m_cursorTex, desktopTexture2d);
    // 【资源释放】9.2 拷贝完数据后，释放桌面纹理
    desktopTexture2d->Release();

    DrawCursor();

    deviceContext->CopyResource(m_texture2dV[idx], m_cursorTex);

    // 【资源释放】9.3 需要释放帧，对应AcquireNextFrame
    m_outputDupV[idx]->ReleaseFrame();

    Save2File(idx, deviceContext, m_texture2dV[idx]);

    result = true;
    return m_texture2dV[idx];
}

bool DXGIDupMgr::DrawCursor()
{
    if (!m_cursorCap) {
        return true;
    }

    IDXGISurface1* surface;
    HRESULT hr = m_cursorTex->QueryInterface(IID_PPV_ARGS(&surface));
    if (FAILED(hr)) {
        return false;
    }

    CURSORINFO cursorInfo = { 0 };
    cursorInfo.cbSize = sizeof(cursorInfo);

    if (GetCursorInfo(&cursorInfo)) {
        if (cursorInfo.flags == CURSOR_SHOWING) {
            auto pos = cursorInfo.ptScreenPos;

            HDC  hdc;
            hr = surface->GetDC(FALSE, &hdc);
            bool ret = DrawIconEx(hdc, pos.x, pos.y, cursorInfo.hCursor,
                0, 0, 0, 0, DI_NORMAL | DI_DEFAULTSIZE);
            hr = surface->ReleaseDC(nullptr);
        }
    }

    return true;
}

void DXGIDupMgr::Save2File(int idx, ID3D11DeviceContext* deviceContext, ID3D11Texture2D* texture)
{
    if (!m_save2file) {
        return;
    }
    UINT height = GetImageHeight(idx);

    int destSize = height * GetImageWidth(idx) * 4;
    BYTE* destImage = new BYTE[destSize];

    // 8.3 纹理映射(GPU -> CPU)
    D3D11_MAPPED_SUBRESOURCE resource;
    UINT subresource = D3D11CalcSubresource(0, 0, 0);
    deviceContext->Map(texture, subresource, D3D11_MAP_READ, 0, &resource);
    if (!resource.pData) {
        return;
    }    
    UINT rowPitch = resource.RowPitch;   
    UINT size = height * resource.RowPitch;
    memcpy_s(destImage, destSize, reinterpret_cast<BYTE*>(resource.pData), size);

    deviceContext->Unmap(texture, subresource);

    // 保持到文件
    static int index = 0;
    char fileName[MAX_PATH] = { 0 };
    CreateDirectory(L"out/bmp", NULL);
    sprintf_s(fileName, "out/bmp/%d_%d.bmp", idx, index++);
    saveBitmap(destImage, rowPitch, height, fileName);

    delete [] destImage;
}