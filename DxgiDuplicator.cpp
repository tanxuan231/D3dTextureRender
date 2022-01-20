// DxgiDuplicator.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "DxgiDuplicator.h"
#include "cursorHelp.h"
#include "Common.h"
#include "log.h"

#pragma comment(lib, "DXGI.lib")

using namespace std;

DXGIDupMgr::DXGIDupMgr()
{
    m_cursorTex = nullptr;

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

ID3D11Texture2D* DXGIDupMgr::GetFrame(int idx, ID3D11Device* device, ID3D11DeviceContext* deviceContext, bool& result)
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
        m_outputDupV[idx]->ReleaseFrame();
        return nullptr;
    }

    // 8. 将桌面图像拷贝出来
    // 8.2 复制纹理(GPU间复制)    
    deviceContext->CopyResource(m_cursorTex, desktopTexture2d);

    //Cursor2Texture(device, deviceContext);
    //DrawCursor();    

    deviceContext->CopyResource(m_texture2dV[idx], desktopTexture2d);

    // 【资源释放】9.2 拷贝完数据后，释放桌面纹理
    desktopTexture2d->Release();
    // 【资源释放】9.3 需要释放帧，对应AcquireNextFrame
    m_outputDupV[idx]->ReleaseFrame();

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

bool DXGIDupMgr::Cursor2Texture(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
    CURSORINFO cursorInfo = { 0 };
    cursorInfo.cbSize = sizeof(cursorInfo);

    if (!GetCursorInfo(&cursorInfo)) {
        return false;
    }

    HICON icon = CopyIcon(cursorInfo.hCursor);
    ICONINFO iconInfo;
    if (!GetIconInfo(icon, &iconInfo)) {
        return false;
    }

    bool ret = true;
    do {
        uint32_t width;
        uint32_t height;
        uint8_t* bitmap = cursor_capture_icon_bitmap(&iconInfo, &width, &height);
        if (!bitmap) {
            ret = false;
            break;
        }

        if (m_cursorTexture == nullptr) {
            D3D11_TEXTURE2D_DESC texture2dDesc;
            texture2dDesc.Width = width;
            texture2dDesc.Height = height;
            texture2dDesc.Format = DXGI_FORMAT_B8G8R8A8_TYPELESS;
            texture2dDesc.MipLevels = 1;
            texture2dDesc.ArraySize = 1;
            texture2dDesc.SampleDesc.Count = 1;
            texture2dDesc.SampleDesc.Quality = 0;
            texture2dDesc.MiscFlags = 0;
#if 1
            texture2dDesc.BindFlags = 0;
            // 该纹理数据从CPU拷贝而来，需要D3D11_CPU_ACCESS_WRITE属性
            // 另外，它还要拷贝到CPU里去，需要D3D11_CPU_ACCESS_READ属性
            texture2dDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
            texture2dDesc.Usage = D3D11_USAGE_STAGING;  // CPU <=> GPU            
            HRESULT hr = device->CreateTexture2D(&texture2dDesc, NULL, &m_cursorTexture);
            if (FAILED(hr)) {
                return false;
            }
#else
            // 该纹理数据从CPU拷贝而来，需要D3D11_CPU_ACCESS_WRITE属性
            // 另外，它需要进行渲染，需要绑定渲染管线，需要D3D11_BIND_SHADER_RESOURCE属性
            texture2dDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            texture2dDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            texture2dDesc.Usage = D3D11_USAGE_DYNAMIC;  // CPU => GPU             
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            srvDesc.Format = texture2dDesc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = 1;

            // 为纹理创建着色器资源视图
            hr = device->CreateShaderResourceView(m_cursorTexture, &srvDesc, &m_cursorTextureView);
            if (FAILED(hr)) {
                return false;
            }
#endif
        }

        uint32_t lineSize = width * 4;

        D3D11_MAPPED_SUBRESOURCE resource;
        UINT subresource = D3D11CalcSubresource(0, 0, 0);
        deviceContext->Map(m_cursorTexture, subresource, D3D11_MAP_READ_WRITE, 0, &resource);
        uint32_t copySize = lineSize > resource.RowPitch ? resource.RowPitch : lineSize;
        memcpy_s(resource.pData, copySize * height, bitmap, width*height*4);        
        deviceContext->Unmap(m_cursorTexture, subresource);

        //SaveTex2File(222, deviceContext, m_cursorTexture, width, height);
    } while (false);

    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);

    return ret;
}