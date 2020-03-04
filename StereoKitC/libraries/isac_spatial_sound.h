#pragma once

// This spatial sound implementation only works on Windows
#ifdef _MSC_VER
#include <mmdeviceapi.h>
#include <SpatialAudioClient.h>
#include <winrt/Windows.Media.Devices.h>
#include <wrl.h>
#include "../stereokit.h"

typedef void (*isac_callback)(float** sourceBuffers, uint32_t numSources, uint32_t numframes, sk::vec3* positions, float* volumes);

class IsacAdapter final
{
public:
    IsacAdapter(int maxSources);
    virtual ~IsacAdapter();

    HRESULT Activate(isac_callback callback);
    static DWORD WINAPI SpatialAudioClientWorker(LPVOID lpParam);

private:
    HRESULT ActivateIsacInterface(winrt::hstring* activatedDeviceId);
    HRESULT ActivateSpatialAudioStream(const WAVEFORMATEX& objectFormat, UINT32 maxObjects);
    HRESULT HandleDeviceChange(winrt::hstring newDeviceId);

private:
    Microsoft::WRL::ComPtr<ISpatialAudioObject>* m_Sources;
    Microsoft::WRL::ComPtr<ISpatialAudioClient> m_Isac;
    Microsoft::WRL::ComPtr<ISpatialAudioObjectRenderStream> m_SpatialAudioStream;
    winrt::hstring m_DeviceIdInUse;
    bool m_IsActivated;
    winrt::event_token m_DeviceChangeToken;
    int m_MaxSources;
    HANDLE m_PumpEvent;
    HANDLE m_PumpThread;
    isac_callback m_AppCallback;
};

class IsacActivator final : public Microsoft::WRL::RuntimeClass<
    Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::ClassicCom>,
    Microsoft::WRL::FtmBase, IActivateAudioInterfaceCompletionHandler>
{
public:
    IsacActivator();
    virtual ~IsacActivator();

    // IActivateAudioInterfaceCompletionHandler
    STDMETHOD(ActivateCompleted)(IActivateAudioInterfaceAsyncOperation* operation);

    // Other
    STDMETHOD(Wait)(DWORD Timeout);
    STDMETHOD(GetActivateResult)(ISpatialAudioClient** deviceAccess);

private:
    HANDLE m_CompletedEvent;
    HRESULT m_ActivateResult;
    Microsoft::WRL::ComPtr<ISpatialAudioClient> m_Isac;
};

#endif