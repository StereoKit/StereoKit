#pragma once

// This spatial sound implementation only works on Windows
#ifdef _MSC_VER
#include <mmdeviceapi.h>
#include <SpatialAudioClient.h>
#include <winrt/Windows.Media.Devices.h>
#include <wrl.h>
// Pull in stereokit.h for vec3 definition
#include "../stereokit.h"

// Typedef to help manage the callback that stereokit gets to fill in audio data
typedef void (*isac_callback)(float** sourceBuffers, uint32_t numSources, uint32_t numframes, sk::vec3* positions, float* volumes);

class IsacAdapter final
{
public:
    IsacAdapter(uint32_t maxSources);
    virtual ~IsacAdapter();

    HRESULT Activate(isac_callback callback);
    static DWORD WINAPI SpatialAudioClientWorker(LPVOID lpParam);

private:
    HRESULT ActivateIsacInterface(winrt::hstring* activatedDeviceId);
    HRESULT ActivateSpatialAudioStream(const WAVEFORMATEX& objectFormat, UINT32 maxObjects);
    HRESULT HandleDeviceChange(winrt::hstring newDeviceId);
    void Stop();

private:
    Microsoft::WRL::ComPtr<ISpatialAudioObject>* m_Sources;
    Microsoft::WRL::ComPtr<ISpatialAudioClient> m_Isac;
    Microsoft::WRL::ComPtr<ISpatialAudioObjectRenderStream> m_SpatialAudioStream;
    winrt::hstring m_DeviceIdInUse;
    bool m_IsActivated;
    winrt::event_token m_DeviceChangeToken;
    uint32_t m_MaxSources;
    HANDLE m_PumpEvent;
    HANDLE m_PumpThread;
    HANDLE m_PumpThreadCanceller;
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