#include "VirtualCam.h"

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
    return DllEntryPoint(reinterpret_cast<HINSTANCE>(hModule), reason, reserved);
}

extern "C" __declspec(dllexport) HRESULT __stdcall DllRegisterServer()
{
    HRESULT hr = AMovieDllRegisterServer2(TRUE);
    if (FAILED(hr))
        return hr;

    IFilterMapper2* mapper = nullptr;
    hr = CoCreateInstance(
        CLSID_FilterMapper2,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IFilterMapper2,
        reinterpret_cast<void**>(&mapper)
    );
    if (FAILED(hr))
        return hr;

    REGFILTER2 rf2;
    ZeroMemory(&rf2, sizeof(rf2));
    rf2.dwVersion = 1;
    rf2.dwMerit = MERIT_DO_NOT_USE;
    rf2.cPins = 1;
    rf2.rgPins = &sudOutputPin;

    hr = mapper->RegisterFilter(
        CLSID_ZigVirtualCamera,
        g_filterName,
        nullptr,
        &CLSID_VideoInputDeviceCategory,
        nullptr,
        &rf2
    );

    mapper->Release();
    return hr;
}

extern "C" __declspec(dllexport) HRESULT __stdcall DllUnregisterServer()
{
    IFilterMapper2* mapper = nullptr;
    HRESULT hr = CoCreateInstance(
        CLSID_FilterMapper2,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IFilterMapper2,
        reinterpret_cast<void**>(&mapper)
    );

    if (SUCCEEDED(hr))
    {
        mapper->UnregisterFilter(
            &CLSID_VideoInputDeviceCategory,
            nullptr,
            CLSID_ZigVirtualCamera
        );
        mapper->Release();
    }

    return AMovieDllRegisterServer2(FALSE);
}
