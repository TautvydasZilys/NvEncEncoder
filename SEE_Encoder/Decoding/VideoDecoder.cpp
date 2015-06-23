#include "PrecompiledHeader.h"
#include "MediaFoundationContext.h"
#include "Utilities\Com\ComClass.h"
#include "Utilities\CriticalSection.h"
#include "VideoDecoder.h"

using namespace Decoding;
using namespace Utilities::Com;

class DecoderPresenter :
	ComClass<IMFTopologyServiceLookupClient, IMFVideoDeviceID, IMFVideoPresenter>
{
private:
	enum class State
	{
		Shutdown,
		Stopped,
		Playing,
		Paused
	};

	Utilities::CriticalSection m_CriticalSection;
	State m_State;
	ComPtr<IMFClock> m_Clock;
	ComPtr<IMediaEventSink> m_MediaSink;
	ComPtr<IMFMediaType> m_MediaType;

	inline void Flush()
	{
		// NOT IMPLEMENTED
	}

public:
	virtual HRESULT STDMETHODCALLTYPE InitServicePointers(IMFTopologyServiceLookup *pLookup) override
	{
		Utilities::CriticalSection::Lock lock(m_CriticalSection);

		if (m_State == State::Playing)
			return MF_E_INVALIDREQUEST;

		DWORD objectCount = 1;
		
		// optional to succeed
		pLookup->LookupService(MF_SERVICE_LOOKUP_GLOBAL, 0, MR_VIDEO_RENDER_SERVICE, __uuidof(IMFClock), &m_Clock, &objectCount);

		objectCount = 1;
		auto hr = pLookup->LookupService(MF_SERVICE_LOOKUP_GLOBAL, 0, MR_VIDEO_RENDER_SERVICE, __uuidof(IMediaEventSink), &m_MediaSink, &objectCount);
		Assert(SUCCEEDED(hr));
		
		m_State = State::Stopped;
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE ReleaseServicePointers() override
	{
		Utilities::CriticalSection::Lock lock(m_CriticalSection);

		Flush();

		m_Clock = nullptr;
		m_MediaSink = nullptr;
		m_State = State::Shutdown;
	}

	virtual HRESULT STDMETHODCALLTYPE GetDeviceID(IID* pDeviceID) override
	{
		*pDeviceID = __uuidof(IDirect3DDevice9);
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE ProcessMessage(MFVP_MESSAGE_TYPE eMessage, ULONG_PTR ulParam) override
	{
		switch (eMessage)
		{
		case MFVP_MESSAGE_INVALIDATEMEDIATYPE:
			Assert(false);
			return E_NOTIMPL;

		case MFVP_MESSAGE_BEGINSTREAMING:
			Assert(false);
			return E_NOTIMPL;

		case MFVP_MESSAGE_ENDSTREAMING:
			Assert(false);
			return E_NOTIMPL;

		case MFVP_MESSAGE_PROCESSINPUTNOTIFY:
			Assert(false);
			return E_NOTIMPL;

		case MFVP_MESSAGE_ENDOFSTREAM:
			Assert(false);
			return E_NOTIMPL;

		case MFVP_MESSAGE_FLUSH:
			Assert(false);
			return E_NOTIMPL;

		case MFVP_MESSAGE_STEP:
			Assert(false);
			return E_NOTIMPL;

		case MFVP_MESSAGE_CANCELSTEP:
			Assert(false);
			return E_NOTIMPL;

		default:
			Assert(false);
			return E_NOTIMPL;
		}

		UNREFERENCED(eMessage);
		UNREFERENCED(ulParam);
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetCurrentMediaType(IMFVideoMediaType** ppMediaType) override
	{
		*ppMediaType = nullptr;

		Utilities::CriticalSection::Lock lock(m_CriticalSection);

		if (m_MediaType == nullptr)
			return MF_E_NOT_INITIALIZED;

		if (m_State == State::Shutdown)
			return MF_E_SHUTDOWN;

		return m_MediaType->QueryInterface(ppMediaType);
	}

	virtual HRESULT STDMETHODCALLTYPE OnClockStart(MFTIME hnsSystemTime, LONGLONG llClockStartOffset) override
	{
		Assert(false);
		UNREFERENCED(hnsSystemTime);
		UNREFERENCED(llClockStartOffset);
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE OnClockStop(MFTIME hnsSystemTime) override
	{
		Assert(false);
		UNREFERENCED(hnsSystemTime);
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE OnClockPause(MFTIME hnsSystemTime) override
	{
		Assert(false);
		UNREFERENCED(hnsSystemTime);
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE OnClockRestart(MFTIME hnsSystemTime) override
	{
		Assert(false);
		UNREFERENCED(hnsSystemTime);
		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE OnClockSetRate(MFTIME hnsSystemTime, float flRate) override
	{
		Assert(false);
		UNREFERENCED(hnsSystemTime);
		UNREFERENCED(flRate);
		return E_NOTIMPL;
	}
};

VideoDecoder::VideoDecoder(const MediaFoundationContext& mfContext, const wchar_t* path)
{
	const DWORD kSourceFlags = MF_RESOLUTION_MEDIASOURCE | MF_RESOLUTION_CONTENT_DOES_NOT_HAVE_TO_MATCH_EXTENSION_OR_MIME_TYPE;
	MF_OBJECT_TYPE objectType;
	ComPtr<IMFSourceResolver> sourceResolver;
	ComPtr<IUnknown> mediaSource;
	ComPtr<IMFTopology> topology;

	auto hr = mfContext->MFCreateMediaSession(nullptr, &m_Session);
	Assert(SUCCEEDED(hr));

	hr = mfContext->MFCreateSourceResolver(&sourceResolver);
	Assert(SUCCEEDED(hr));

	hr = sourceResolver->CreateObjectFromURL(path, kSourceFlags, nullptr, &objectType, &mediaSource);
	Assert(SUCCEEDED(hr));
	
	hr = mediaSource.As(&m_MediaSource);
	Assert(SUCCEEDED(hr));

	hr = mfContext->MFCreateTopology(&topology);
	Assert(SUCCEEDED(hr));


}

VideoDecoder::~VideoDecoder()
{
}