#include "PrecompiledHeader.h"
#include "MediaFoundationContext.h"
#include "Utilities\Logging.h"

using namespace Decoding;

MediaFoundationContext::MediaFoundationContext(Utilities::Logging& logging) :
	m_MfPlatDll(nullptr),
	m_MfDll(nullptr)
{
	ZeroStructure(&m_FunctionTable);

	m_MfPlatDll = LoadLibraryW(L"MfPlat.dll");

	if (m_MfPlatDll == nullptr)
	{
		logging.Error("ERROR: Failed to load MfPlat.dll.");
		throw std::runtime_error("Failed to load MfPlat.dll.");
	}

	m_MfDll = LoadLibraryW(L"Mf.dll");

	if (m_MfDll == nullptr)
	{
		logging.Error("ERROR: Failed to load Mf.dll.");
		throw std::runtime_error("Failed to load Mf.dll.");
	}
	
	m_FunctionTable.MFStartup = reinterpret_cast<MediaFoundationFunctions::MFStartupFunc>(GetProcAddress(m_MfPlatDll, "MFStartup"));
	Assert(m_FunctionTable.MFStartup != nullptr);

	m_FunctionTable.MFShutdown = reinterpret_cast<MediaFoundationFunctions::MFShutdownFunc>(GetProcAddress(m_MfPlatDll, "MFShutdown"));
	Assert(m_FunctionTable.MFShutdown != nullptr);

	m_FunctionTable.MFCreateSourceResolver = reinterpret_cast<MediaFoundationFunctions::MFCreateSourceResolverFunc>(GetProcAddress(m_MfPlatDll, "MFCreateSourceResolver"));
	Assert(m_FunctionTable.MFCreateSourceResolver != nullptr);

	m_FunctionTable.MFCreateMediaSession = reinterpret_cast<MediaFoundationFunctions::MFCreateMediaSessionFunc>(GetProcAddress(m_MfDll, "MFCreateMediaSession"));
	Assert(m_FunctionTable.MFCreateMediaSession != nullptr);

	m_FunctionTable.MFCreateTopology = reinterpret_cast<MediaFoundationFunctions::MFCreateTopologyFunc>(GetProcAddress(m_MfDll, "MFCreateTopology"));
	Assert(m_FunctionTable.MFCreateTopology != nullptr);

	auto hr = m_FunctionTable.MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	Assert(SUCCEEDED(hr));	
}

MediaFoundationContext::~MediaFoundationContext()
{
	auto hr = m_FunctionTable.MFShutdown();
	Assert(SUCCEEDED(hr));

	auto result = FreeLibrary(m_MfDll);
	Assert(result != FALSE);

	result = FreeLibrary(m_MfPlatDll);
	Assert(result != FALSE);
}
