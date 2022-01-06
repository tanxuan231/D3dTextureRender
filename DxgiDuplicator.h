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

    void EnableSave2File() { m_save2file = true; }
    void EnableCursorCap() { m_cursorCap = true; }

    UINT GetImageHeight(int idx) {
        DXGI_OUTDUPL_DESC desc;
        m_outputDupV[idx]->GetDesc(&desc);
        return desc.ModeDesc.Height;
    }

    UINT GetImageWidth(int idx) {
        DXGI_OUTDUPL_DESC desc;
        m_outputDupV[idx]->GetDesc(&desc);
        return desc.ModeDesc.Width;
    }

    DXGI_FORMAT GetImageFormat(int idx) {
        DXGI_OUTDUPL_DESC desc;
        m_outputDupV[idx]->GetDesc(&desc);
        return desc.ModeDesc.Format;
    }

    ID3D11Texture2D* GetFrame(int idx, ID3D11Device* device, ID3D11DeviceContext* deviceContext, bool& result);

private:
    bool InitOutput(int monitorIdx, ID3D11Device* device, IDXGIAdapter* dxgiAdapter);    
    bool CreateTexture(ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format);    
    void SaveTex2File(int idx, ID3D11DeviceContext* deviceContext, 
        ID3D11Texture2D* texture, UINT width, UINT height, bool force = false);
    bool DrawCursor();
    bool Cursor2Texture(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

private:
    bool m_save2file;
    bool m_cursorCap;

private:
    std::vector<IDXGIOutput*> m_dxgiOutputV;
    std::vector<IDXGIOutput1*> m_dxgiOutput1V;
    std::vector<IDXGIOutputDuplication*> m_outputDupV;
    std::vector<ID3D11Texture2D*> m_texture2dV;

    ID3D11Texture2D* m_cursorTex;

    ID3D11Texture2D* m_cursorTexture = nullptr;   // π‚±ÍŒ∆¿Ì
    ID3D11ShaderResourceView* m_cursorTextureView;

    // new add
    ID3D11Texture2D* m_obsTex = nullptr;
    ID3D11ShaderResourceView* m_obsTexView;
};