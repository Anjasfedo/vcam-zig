#include "VirtualCam.h"
#include <cmath>

const WCHAR g_filterName[] = L"Zig Virtual Camera";

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_Video,
    &MEDIASUBTYPE_RGB24
};

const AMOVIESETUP_PIN sudOutputPin =
{
    L"Output",
    FALSE,              // rendered
    TRUE,               // output
    FALSE,              // zero instances
    FALSE,              // many instances
    &CLSID_NULL,
    nullptr,
    1,
    &sudPinTypes
};

const AMOVIESETUP_FILTER sudFilter =
{
    &CLSID_ZigVirtualCamera,
    g_filterName,
    MERIT_DO_NOT_USE,
    1,
    &sudOutputPin
};

CFactoryTemplate g_Templates[] =
{
    {
        g_filterName,
        &CLSID_ZigVirtualCamera,
        CZigVirtualCam::CreateInstance,
        nullptr,
        &sudFilter
    }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

CUnknown* WINAPI CZigVirtualCam::CreateInstance(IUnknown* pUnk, HRESULT* phr)
{
    CZigVirtualCam* filter = new CZigVirtualCam(pUnk, phr);
    if (!filter && phr)
        *phr = E_OUTOFMEMORY;
    return filter;
}

CZigVirtualCam::CZigVirtualCam(IUnknown* pUnk, HRESULT* phr)
    : CSource(g_filterName, pUnk, CLSID_ZigVirtualCamera)
{
    CAutoLock lock(&m_cStateLock);
    CZigVirtualCamStream* stream = new CZigVirtualCamStream(phr, this);
    if (!stream && phr)
        *phr = E_OUTOFMEMORY;
}

CZigVirtualCamStream::CZigVirtualCamStream(HRESULT* phr, CZigVirtualCam* parent)
    : CSourceStream(L"Zig Virtual Camera Stream", phr, parent, L"Output")
{
}

STDMETHODIMP CZigVirtualCamStream::QueryInterface(REFIID riid, void** ppv)
{
    if (riid == IID_IAMStreamConfig)
        return GetInterface(static_cast<IAMStreamConfig*>(this), ppv);
    if (riid == IID_IKsPropertySet)
        return GetInterface(static_cast<IKsPropertySet*>(this), ppv);
    return CSourceStream::QueryInterface(riid, ppv);
}

STDMETHODIMP_(ULONG) CZigVirtualCamStream::AddRef()
{
    return CSourceStream::AddRef();
}

STDMETHODIMP_(ULONG) CZigVirtualCamStream::Release()
{
    return CSourceStream::Release();
}

void CZigVirtualCamStream::FillBitmapInfo(VIDEOINFOHEADER* vih)
{
    ZeroMemory(vih, sizeof(VIDEOINFOHEADER));

    vih->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    vih->bmiHeader.biWidth = m_width;
    vih->bmiHeader.biHeight = m_height; // positive means bottom-up DIB
    vih->bmiHeader.biPlanes = 1;
    vih->bmiHeader.biBitCount = 24;
    vih->bmiHeader.biCompression = BI_RGB;
    vih->bmiHeader.biSizeImage = m_width * m_height * 3;
    vih->AvgTimePerFrame = m_rtFrameLength;

    SetRect(&vih->rcSource, 0, 0, m_width, m_height);
    SetRect(&vih->rcTarget, 0, 0, m_width, m_height);
}

HRESULT CZigVirtualCamStream::GetMediaType(CMediaType* mediaType)
{
    CAutoLock lock(m_pFilter->pStateLock());

    mediaType->InitMediaType();
    mediaType->SetType(&MEDIATYPE_Video);
    mediaType->SetSubtype(&MEDIASUBTYPE_RGB24);
    mediaType->SetFormatType(&FORMAT_VideoInfo);
    mediaType->SetTemporalCompression(FALSE);
    mediaType->SetSampleSize(m_width * m_height * 3);

    VIDEOINFOHEADER* vih = reinterpret_cast<VIDEOINFOHEADER*>(
        mediaType->AllocFormatBuffer(sizeof(VIDEOINFOHEADER))
    );
    if (!vih)
        return E_OUTOFMEMORY;

    FillBitmapInfo(vih);
    return S_OK;
}

HRESULT CZigVirtualCamStream::CheckMediaType(const CMediaType* mediaType)
{
    if (*mediaType->Type() != MEDIATYPE_Video)
        return E_INVALIDARG;
    if (*mediaType->Subtype() != MEDIASUBTYPE_RGB24)
        return E_INVALIDARG;
    if (*mediaType->FormatType() != FORMAT_VideoInfo)
        return E_INVALIDARG;
    return S_OK;
}

HRESULT CZigVirtualCamStream::SetMediaType(const CMediaType* mediaType)
{
    HRESULT hr = CSourceStream::SetMediaType(mediaType);
    if (FAILED(hr))
        return hr;

    VIDEOINFOHEADER* vih = reinterpret_cast<VIDEOINFOHEADER*>(mediaType->Format());
    if (vih)
    {
        m_width = vih->bmiHeader.biWidth;
        m_height = abs(vih->bmiHeader.biHeight);
        if (vih->AvgTimePerFrame > 0)
            m_rtFrameLength = vih->AvgTimePerFrame;
    }
    return S_OK;
}

HRESULT CZigVirtualCamStream::DecideBufferSize(IMemAllocator* allocator, ALLOCATOR_PROPERTIES* properties)
{
    CAutoLock lock(m_pFilter->pStateLock());

    properties->cBuffers = 1;
    properties->cbBuffer = m_width * m_height * 3;

    ALLOCATOR_PROPERTIES actual;
    HRESULT hr = allocator->SetProperties(properties, &actual);
    if (FAILED(hr))
        return hr;
    if (actual.cbBuffer < properties->cbBuffer)
        return E_FAIL;
    return S_OK;
}

HRESULT CZigVirtualCamStream::FillBuffer(IMediaSample* sample)
{
    BYTE* dst = nullptr;
    HRESULT hr = sample->GetPointer(&dst);
    if (FAILED(hr) || !dst)
        return hr;

    const int stride = m_width * 3;

    // DirectShow RGB24 DIB stores B, G, R bytes. This generates a moving gradient.
    for (int y = 0; y < m_height; ++y)
    {
        BYTE* row = dst + y * stride;
        for (int x = 0; x < m_width; ++x)
        {
            int i = x * 3;
            row[i + 0] = static_cast<BYTE>((x + m_frame) & 0xff);       // B
            row[i + 1] = static_cast<BYTE>((y + m_frame) & 0xff);       // G
            row[i + 2] = static_cast<BYTE>((x + y + m_frame) & 0xff);   // R
        }
    }

    REFERENCE_TIME start = m_frame * m_rtFrameLength;
    REFERENCE_TIME end = start + m_rtFrameLength;

    sample->SetTime(&start, &end);
    sample->SetSyncPoint(TRUE);
    sample->SetActualDataLength(m_width * m_height * 3);

    ++m_frame;
    return S_OK;
}

STDMETHODIMP CZigVirtualCamStream::SetFormat(AM_MEDIA_TYPE* pmt)
{
    if (!pmt)
        return E_POINTER;
    CMediaType mt(*pmt);
    return SetMediaType(&mt);
}

STDMETHODIMP CZigVirtualCamStream::GetFormat(AM_MEDIA_TYPE** ppmt)
{
    if (!ppmt)
        return E_POINTER;
    CMediaType mt;
    HRESULT hr = GetMediaType(&mt);
    if (FAILED(hr))
        return hr;
    *ppmt = CreateMediaType(&mt);
    return *ppmt ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CZigVirtualCamStream::GetNumberOfCapabilities(int* piCount, int* piSize)
{
    if (!piCount || !piSize)
        return E_POINTER;
    *piCount = 1;
    *piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS);
    return S_OK;
}

STDMETHODIMP CZigVirtualCamStream::GetStreamCaps(int iIndex, AM_MEDIA_TYPE** ppmt, BYTE* pSCC)
{
    if (iIndex != 0)
        return S_FALSE;
    if (!ppmt || !pSCC)
        return E_POINTER;

    CMediaType mt;
    HRESULT hr = GetMediaType(&mt);
    if (FAILED(hr))
        return hr;
    *ppmt = CreateMediaType(&mt);
    hr = *ppmt ? S_OK : E_OUTOFMEMORY;
    if (FAILED(hr))
        return hr;

    VIDEO_STREAM_CONFIG_CAPS* caps = reinterpret_cast<VIDEO_STREAM_CONFIG_CAPS*>(pSCC);
    ZeroMemory(caps, sizeof(VIDEO_STREAM_CONFIG_CAPS));

    caps->guid = FORMAT_VideoInfo;
    caps->VideoStandard = AnalogVideo_None;
    caps->InputSize.cx = m_width;
    caps->InputSize.cy = m_height;
    caps->MinCroppingSize.cx = m_width;
    caps->MinCroppingSize.cy = m_height;
    caps->MaxCroppingSize.cx = m_width;
    caps->MaxCroppingSize.cy = m_height;
    caps->CropGranularityX = 1;
    caps->CropGranularityY = 1;
    caps->CropAlignX = 1;
    caps->CropAlignY = 1;
    caps->MinOutputSize.cx = m_width;
    caps->MinOutputSize.cy = m_height;
    caps->MaxOutputSize.cx = m_width;
    caps->MaxOutputSize.cy = m_height;
    caps->OutputGranularityX = 1;
    caps->OutputGranularityY = 1;
    caps->MinFrameInterval = m_rtFrameLength;
    caps->MaxFrameInterval = m_rtFrameLength;
    caps->MinBitsPerSecond = m_width * m_height * 3 * 8 * m_fps;
    caps->MaxBitsPerSecond = m_width * m_height * 3 * 8 * m_fps;

    return S_OK;
}

STDMETHODIMP CZigVirtualCamStream::Set(REFGUID, DWORD, void*, DWORD, void*, DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP CZigVirtualCamStream::Get(
    REFGUID guidPropSet,
    DWORD dwPropID,
    void*,
    DWORD,
    void* pPropData,
    DWORD cbPropData,
    DWORD* pcbReturned
)
{
    if (guidPropSet == AMPROPSETID_Pin && dwPropID == AMPROPERTY_PIN_CATEGORY)
    {
        if (pcbReturned)
            *pcbReturned = sizeof(GUID);
        if (pPropData == nullptr)
            return S_OK;
        if (cbPropData < sizeof(GUID))
            return E_UNEXPECTED;

        *reinterpret_cast<GUID*>(pPropData) = PIN_CATEGORY_CAPTURE;
        return S_OK;
    }
    return E_PROP_ID_UNSUPPORTED;
}

STDMETHODIMP CZigVirtualCamStream::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD* pTypeSupport)
{
    if (!pTypeSupport)
        return E_POINTER;

    if (guidPropSet == AMPROPSETID_Pin && dwPropID == AMPROPERTY_PIN_CATEGORY)
    {
        *pTypeSupport = KSPROPERTY_SUPPORT_GET;
        return S_OK;
    }
    return E_PROP_ID_UNSUPPORTED;
}

