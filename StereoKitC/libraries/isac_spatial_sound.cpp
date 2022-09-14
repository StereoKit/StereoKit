#ifdef _MSC_VER

#include "isac_spatial_sound.h"
#include <winrt/Windows.Foundation.h>
#include <mmdeviceapi.h>
#include <SpatialAudioClient.h>
#include <winrt/Windows.Media.Devices.h>
#include <wrl.h>

using namespace Microsoft::WRL;
using namespace winrt::Windows::Media::Devices;
using namespace sk;

///////////////////////////////////////////

constexpr uint32_t c_HrtfSampleRate = 48000;

///////////////////////////////////////////

// Some helper macros to deal with HRESULTs and allocations
#define RETURN_IF_NULL_ALLOC(x) \
if (x == nullptr) return E_OUTOFMEMORY

#define RETURN_IF_FAILED(x) \
{HRESULT __hr = x; \
if (FAILED(__hr)) return __hr;}

#define RETURN_HR_IF(x, y) \
if (y) return x

#define RETURN_HR_IF_NULL(x, y) \
if (y == nullptr) return x

///////////////////////////////////////////

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

///////////////////////////////////////////

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

///////////////////////////////////////////

IsacAdapter* isac_adapter = nullptr;

long isac_activate(uint32_t maxSources, isac_callback callback) {
	isac_adapter = new IsacAdapter(maxSources);
	HRESULT hr = isac_adapter->Activate(callback);

	if (FAILED(hr)) {
		delete isac_adapter;
		isac_adapter = nullptr;
	}
	return hr;
}

///////////////////////////////////////////

void isac_destroy() {
	if (isac_adapter)
		delete isac_adapter;
}

///////////////////////////////////////////
// IsacAdapter implementation -----------------------

IsacAdapter::IsacAdapter(uint32_t maxSources) : m_IsActivated(false), 
	m_PumpEvent(nullptr), m_AppCallback(nullptr), m_PumpThread(nullptr)
{
	// Subscribe to device change notifications so we can reactivate later
	winrt::Windows::Foundation::TypedEventHandler<struct winrt::Windows::Foundation::IInspectable, 
		struct winrt::Windows::Media::Devices::DefaultAudioRenderDeviceChangedEventArgs> 
		deviceChangeHandler = [&](winrt::Windows::Foundation::IInspectable const&,
			DefaultAudioRenderDeviceChangedEventArgs const& args) {
			return HandleDeviceChange(args.Id());
	};
	m_DeviceChangeToken = MediaDevice::DefaultAudioRenderDeviceChanged(deviceChangeHandler);
	m_Sources = new ComPtr<ISpatialAudioObject>[maxSources];
	m_MaxSources = maxSources;
	m_PumpThreadCanceller = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

///////////////////////////////////////////

IsacAdapter::~IsacAdapter() {
	// Unregister from device change notifications
	MediaDevice::DefaultAudioRenderDeviceChanged(m_DeviceChangeToken);
	Stop();
	delete[] m_Sources;
	CloseHandle(m_PumpEvent);
	CloseHandle(m_PumpThreadCanceller);
}

///////////////////////////////////////////

// If ISAC is currently playing audio, stop it and cancel processing on the render thread
void IsacAdapter::Stop()
{
	if (m_SpatialAudioStream)
	{
		m_SpatialAudioStream->Stop();
		SetEvent(m_PumpThreadCanceller);
	}
}

///////////////////////////////////////////

// Calls ActivateAudioInterfaceAsync and waits for it to complete
// On success, sets activatedDeviceId to the id of the device that ISAC is activated against
HRESULT IsacAdapter::ActivateIsacInterface(winrt::hstring* activatedDeviceId) {
	// ActivateAudioInterfaceAsync takes a special activation object.
	// This is our implementation of that object.
	ComPtr<IsacActivator> spCompletionObject = Make<IsacActivator>();
	RETURN_IF_NULL_ALLOC(spCompletionObject);

	ComPtr<IActivateAudioInterfaceCompletionHandler> spCompletionHandler;
	RETURN_IF_FAILED(spCompletionObject.As(&spCompletionHandler));

	winrt::hstring deviceId = MediaDevice::GetDefaultAudioRenderId(AudioDeviceRole::Default);

	// Activate the ISpatialAudioClient interface and wait for it to complete
	ComPtr<IActivateAudioInterfaceAsyncOperation> spOperation;
	RETURN_IF_FAILED(ActivateAudioInterfaceAsync(
		deviceId.c_str(), __uuidof(ISpatialAudioClient), nullptr, spCompletionHandler.Get(), &spOperation));
	RETURN_IF_FAILED(spCompletionObject->Wait(INFINITE));
	RETURN_IF_FAILED(spCompletionObject->GetActivateResult(&m_Isac));

	*activatedDeviceId = deviceId;
	return S_OK;
}

///////////////////////////////////////////

// Given an activated ISpatialAudioClient interface, finds the supported WAVEFORMATEX that matches our needs
// We only operate in 48kHz float. If that's supported, it is returned. Otherwise, this function fails
HRESULT FindAcceptableWaveformat(ISpatialAudioClient* isac, WAVEFORMATEX* objectFormat) {
	// Initialize ISAC with its preferred format
	ComPtr<IAudioFormatEnumerator> audioObjectFormatEnumerator;
	RETURN_IF_FAILED(isac->GetSupportedAudioObjectFormatEnumerator(&audioObjectFormatEnumerator));
	UINT32 audioObjectFormatCount = 0;
	RETURN_IF_FAILED(audioObjectFormatEnumerator->GetCount(&audioObjectFormatCount));
	RETURN_HR_IF(E_FAIL, audioObjectFormatCount == 0);

	// Find the first format that's in 48kHz, float. This is the best supported format
	for (uint32_t i = 0u; i < audioObjectFormatCount; i++) {
		WAVEFORMATEX* format = nullptr;
		RETURN_IF_FAILED(audioObjectFormatEnumerator->GetFormat(0, &format));
		if (format->nSamplesPerSec == c_HrtfSampleRate && format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
			*objectFormat = *format;
			break;
		}
	}
	RETURN_HR_IF_NULL(E_NOT_VALID_STATE, objectFormat);

	return S_OK;
}

///////////////////////////////////////////

// Assuming ISAC has been activated, uses that ISAC in conjunction with the passed-in parameters to activate the
// spatial audio stream and associated static bed objects
HRESULT IsacAdapter::ActivateSpatialAudioStream(const WAVEFORMATEX& objectFormat, UINT32 maxObjects) {
	RETURN_HR_IF_NULL(E_NOT_VALID_STATE, m_Isac);
	if (m_PumpEvent == nullptr) {
		m_PumpEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	}

	// Setup Static Bed Object mask
	AudioObjectType objectMask = AudioObjectType_None;
	SpatialAudioObjectRenderStreamActivationParams activationParams = {
		&objectFormat, objectMask, 0, maxObjects, AudioCategory_GameEffects, m_PumpEvent, nullptr };

	PROPVARIANT activateParams;
	PropVariantInit(&activateParams);
	activateParams.vt = VT_BLOB;
	activateParams.blob.cbSize = sizeof(activationParams);
	activateParams.blob.pBlobData = reinterpret_cast<BYTE*>(&activationParams);
	RETURN_IF_FAILED(m_Isac->ActivateSpatialAudioStream(&activateParams, IID_PPV_ARGS(&m_SpatialAudioStream)));

	return S_OK;
}

///////////////////////////////////////////

// Fully activates, initializes, and starts the ISAC interfaces and all related member variables
HRESULT IsacAdapter::Activate(isac_callback callback) {
	RETURN_HR_IF_NULL(E_INVALIDARG, callback);
	
	m_AppCallback = callback;
	// Nothing to do if already activated
	RETURN_HR_IF(S_OK, m_IsActivated);

	winrt::hstring deviceId;
	RETURN_IF_FAILED(ActivateIsacInterface(&deviceId));

	WAVEFORMATEX objectFormat;
	RETURN_IF_FAILED(FindAcceptableWaveformat(m_Isac.Get(), &objectFormat));

	// Determine how many dynamic objects this platform supports
	// If we can't get at least m_MaxSources, bail out and let stereokit use its fallback audio implementation
	UINT32 maxObjects = 0;
	RETURN_IF_FAILED(m_Isac->GetMaxDynamicObjectCount(&maxObjects));
	RETURN_HR_IF(E_NOT_VALID_STATE, maxObjects == 0 || maxObjects < m_MaxSources);
	RETURN_IF_FAILED(ActivateSpatialAudioStream(objectFormat, maxObjects));

	// It's important to only set the deviceId after successfully initializing everything
	// Otherwise, we may fail a retry on new device arrival notifications
	m_DeviceIdInUse = deviceId;
	m_IsActivated = true;

	// Get the dynamic sources
	for (uint32_t i = 0; i < m_MaxSources; i++) {
		RETURN_IF_FAILED(m_SpatialAudioStream->ActivateSpatialAudioObject(AudioObjectType_Dynamic, &m_Sources[i]));
	}

	RETURN_IF_FAILED(m_SpatialAudioStream->Start());

	// If we previously created a thread, close it down first
	if (m_PumpThread != nullptr) CloseHandle(m_PumpThread);
	// Start the rendering thread
	m_PumpThread = CreateThread(nullptr, 0, &IsacAdapter::SpatialAudioClientWorker, (void*)this, 0, nullptr);
	SetThreadPriority(m_PumpThread, THREAD_PRIORITY_ABOVE_NORMAL);
			
	return S_OK;
}

///////////////////////////////////////////

// Reacts to an audio endpoint change
HRESULT IsacAdapter::HandleDeviceChange(winrt::hstring newDeviceId) {
	// Don't process this device change request if we're already playing on the new device
	RETURN_HR_IF(S_OK, newDeviceId == m_DeviceIdInUse);

	m_IsActivated = false;
	Stop();

	RETURN_IF_FAILED(Activate(m_AppCallback));

	return S_OK;
}

///////////////////////////////////////////

// This must be called on a separate thread, as it never returns (unless cancelled).
// It will run every ISAC pump pass (roughly 10ms), triggered by the Windows audio stack setting the m_PumpEvent
DWORD WINAPI IsacAdapter::SpatialAudioClientWorker(LPVOID lpParam) {
	IsacAdapter* pThis = (IsacAdapter*)lpParam;
	BYTE** objectBuffers = new BYTE*[pThis->m_MaxSources];
	vec3* positions = new vec3[pThis->m_MaxSources];
	float* volumes = new float[pThis->m_MaxSources];
	// Setup the render loop. It will run indefinitely. Every time PumpEvent is set, the while loop continues
	// If PumpThreadCanceller is set, WaitForMultipleObjects will return WAIT_OBJECT_1, and the while loop will break
	const HANDLE eventsToWaitOn[2] = { pThis->m_PumpEvent, pThis->m_PumpThreadCanceller };
	while (WAIT_OBJECT_0 == WaitForMultipleObjects(2, eventsToWaitOn, FALSE, INFINITE)) {
		UINT32 objects = 0;
		UINT32 frameCount = 0;
		if (FAILED(pThis->m_SpatialAudioStream->BeginUpdatingAudioObjects(&objects, &frameCount))) break;

		// Fill the buffers for the call into the app code
		UINT32 byteCount = 0;
		for (uint32_t i = 0; i < pThis->m_MaxSources; i++)         {
			pThis->m_Sources[i]->GetBuffer(&objectBuffers[i], &byteCount);
			// Fill the buffers with 0s
			memset(objectBuffers[i], 0, byteCount);
			// Set all positions and volumes to 0, too. The app callback will overwrite them
			positions[i] = vec3_zero;
			volumes[i] = 0.0f;
		}

		// Notify Stereokit that ISAC needs data. Stereokit will handle filling in each active object's information
		pThis->m_AppCallback((float**)objectBuffers, pThis->m_MaxSources, byteCount / sizeof(float), positions, volumes);

		for (uint32_t i = 0; i < pThis->m_MaxSources; i++) {
			// Intentionally ignore any per-object failures and continue to the next object
			pThis->m_Sources[i]->SetPosition(positions[i].x, positions[i].y, positions[i].z);
			// Apply 1/r gain
			float dist = vec3_magnitude(positions[i]);
			// Guard against objects being too close
			if (dist <= 0.00001f) dist = 1;
			pThis->m_Sources[i]->SetVolume(min(1.0f, volumes[i] / dist));
		}

		pThis->m_SpatialAudioStream->EndUpdatingAudioObjects();
	}

	delete[] objectBuffers;
	delete[] positions;
	delete[] volumes;
	return 0;
}

///////////////////////////////////////////
// IsacActivator implementation -----------------------------

IsacActivator::IsacActivator() : m_ActivateResult(E_ILLEGAL_METHOD_CALL) {
	m_CompletedEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

///////////////////////////////////////////

IsacActivator::~IsacActivator() {
	CloseHandle(m_CompletedEvent);
}

///////////////////////////////////////////

// Called by the Windows audio stack when activating the ISAC interface is completed
// Inspect the contents of operation to see if activation succeeded or not
STDMETHODIMP IsacActivator::ActivateCompleted(IActivateAudioInterfaceAsyncOperation* operation) {
	SetEvent(m_CompletedEvent);
	ComPtr<IUnknown> spAudioAsUnknown;
	RETURN_IF_FAILED(operation->GetActivateResult(&m_ActivateResult, &spAudioAsUnknown));
	RETURN_IF_FAILED(m_ActivateResult);
	m_ActivateResult = spAudioAsUnknown.As(&m_Isac);

	return S_OK;
}

///////////////////////////////////////////

// A helper to let IsacAdapter synchronously wait for the activation to be completed
STDMETHODIMP IsacActivator::Wait(DWORD Timeout) {
	DWORD waitResult = WaitForSingleObject(m_CompletedEvent, Timeout);
	if (waitResult == WAIT_OBJECT_0) return S_OK;
	RETURN_HR_IF(HRESULT_FROM_WIN32(ERROR_TIMEOUT), waitResult == WAIT_TIMEOUT);
	RETURN_HR_IF(HRESULT_FROM_WIN32(GetLastError()), waitResult == WAIT_FAILED);

	return E_FAIL;
}

///////////////////////////////////////////

// Return the result of activating the ISAC interface to the IsacAdapter
STDMETHODIMP IsacActivator::GetActivateResult(ISpatialAudioClient** deviceAccess) {
	*deviceAccess = nullptr;
	RETURN_IF_FAILED(m_ActivateResult);
	RETURN_IF_FAILED(m_Isac.CopyTo(deviceAccess));
	return S_OK;
}

#endif // _MSC_VER