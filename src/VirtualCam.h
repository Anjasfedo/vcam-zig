#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <dshow.h>
#include <streams.h>
#include <initguid.h>

// {3F8D4A2E-9B6A-4F27-8D65-62E1C59C1111}
// Generate your own GUID for production projects.
DEFINE_GUID(CLSID_ZigVirtualCamera,
    0x3f8d4a2e, 0x9b6a, 0x4f27,
    0x8d, 0x65, 0x62, 0xe1, 0xc5, 0x9c, 0x11, 0x11);

extern const WCHAR g_filterName[];
extern const AMOVIESETUP_PIN sudOutputPin;

class CZigVirtualCamStream;

class CZigVirtualCam : public CSource
{
public:
    static CUnknown* WINAPI CreateInstance(IUnknown* pUnk, HRESULT* phr);

private:
    CZigVirtualCam(IUnknown* pUnk, HRESULT* phr);
};

class CZigVirtualCamStream :
    public CSourceStream,
    public IAMStreamConfig,
    public IKsPropertySet
{
public:
    CZigVirtualCamStream(HRESULT* phr, CZigVirtualCam* parent);

    // CSourceStream
    HRESULT FillBuffer(IMediaSample* sample) override;
    HRESULT DecideBufferSize(IMemAllocator* allocator, ALLOCATOR_PROPERTIES* properties) override;
    HRESULT GetMediaType(CMediaType* mediaType) override;
    HRESULT CheckMediaType(const CMediaType* mediaType) override;
    HRESULT SetMediaType(const CMediaType* mediaType) override;

    // CUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    // IAMStreamConfig
    STDMETHODIMP SetFormat(AM_MEDIA_TYPE* pmt) override;
    STDMETHODIMP GetFormat(AM_MEDIA_TYPE** ppmt) override;
    STDMETHODIMP GetNumberOfCapabilities(int* piCount, int* piSize) override;
    STDMETHODIMP GetStreamCaps(int iIndex, AM_MEDIA_TYPE** ppmt, BYTE* pSCC) override;

    // IKsPropertySet
    STDMETHODIMP Set(
        REFGUID guidPropSet,
        DWORD dwPropID,
        void* pInstanceData,
        DWORD cbInstanceData,
        void* pPropData,
        DWORD cbPropData
    ) override;

    STDMETHODIMP Get(
        REFGUID guidPropSet,
        DWORD dwPropID,
        void* pInstanceData,
        DWORD cbInstanceData,
        void* pPropData,
        DWORD cbPropData,
        DWORD* pcbReturned
    ) override;

    STDMETHODIMP QuerySupported(
        REFGUID guidPropSet,
        DWORD dwPropID,
        DWORD* pTypeSupport
    ) override;

private:
    void FillBitmapInfo(VIDEOINFOHEADER* vih);

private:
    int m_width = 640;
    int m_height = 480;
    int m_fps = 30;
    int m_frame = 0;
    REFERENCE_TIME m_rtFrameLength = 333333; // 30 FPS in DirectShow 100ns units.
};
