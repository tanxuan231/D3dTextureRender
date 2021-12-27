#pragma once

#include <dxgi.h>
#include <vector>
#include <d3d11.h>
#include <dxgi1_2.h>

class DXGIDupMgr {
public:
    DXGIDupMgr();
    ~DXGIDupMgr();

    bool Init(ID3D11Device* device, IDXGIAdapter* dxgiAdapter);

    UINT GetImageHeight(int idx) {
        DXGI_OUTDUPL_DESC desc;
        m_outputDupV[idx]->GetDesc(&desc);
        return desc.ModeDesc.Height;
    }

    UINT GetImageWidth(int idx) {
        DXGI_OUTDUPL_DESC desc;
        m_outputDupV[idx]->GetDesc(&desc);
        return desc.ModeDesc.Height;
    }

    ID3D11Texture2D* GetFrame(int idx, ID3D11DeviceContext* deviceContext, void* destImage, UINT destSize, UINT* rowPitch);

private:
    bool InitOutput(int monitorIdx, ID3D11Device* device, IDXGIAdapter* dxgiAdapter);    
    bool CreateTexture(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format);    

private:
    std::vector<IDXGIOutput*> m_dxgiOutputV;
    std::vector<IDXGIOutput1*> m_dxgiOutput1V;
    std::vector<IDXGIOutputDuplication*> m_outputDupV;
    std::vector<ID3D11Texture2D*> m_texture2dV;
};