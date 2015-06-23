#pragma once

namespace Utilities { class Logging; }

namespace Decoding
{
	struct MediaFoundationFunctions
	{
		typedef HRESULT (__stdcall *MFStartupFunc)(ULONG Version, DWORD dwFlags);
		typedef HRESULT (__stdcall *MFShutdownFunc)();
		typedef HRESULT (__stdcall *MFCreateMediaSessionFunc)(IMFAttributes* pConfiguration, IMFMediaSession** ppMediaSession);
		typedef HRESULT (__stdcall *MFCreateSourceResolverFunc)(IMFSourceResolver** ppISourceResolver);
		typedef HRESULT (__stdcall *MFCreateTopologyFunc)(IMFTopology** ppTopology);
		
		MFStartupFunc MFStartup;
		MFShutdownFunc MFShutdown;
		MFCreateMediaSessionFunc MFCreateMediaSession;
		MFCreateSourceResolverFunc MFCreateSourceResolver;
		MFCreateTopologyFunc MFCreateTopology;
	};

	class MediaFoundationContext
	{
	private:
		HMODULE m_MfPlatDll;
		HMODULE m_MfDll;
		MediaFoundationFunctions m_FunctionTable;

	public:
		MediaFoundationContext(Utilities::Logging& logging);
		~MediaFoundationContext();

		MediaFoundationContext(const MediaFoundationContext&) = delete;
		MediaFoundationContext& operator=(const MediaFoundationContext&) = delete;

		inline const MediaFoundationFunctions* operator->() const { return &m_FunctionTable; }
	};
}