#include "PrecompiledHeader.h"
#include "NvEncode.h"
#include "Utilities\Logging.h"

NvEncode::NvEncode() :
	m_NvEncodeApiDll(nullptr)
{
	ZeroStructure(&m_NvEncode);

#if _WIN64
	const wchar_t kNvEncodeApiDllName[] = L"nvEncodeAPI64.dll";
#else
	const wchar_t kNvEncodeApiDllName[] = L"nvEncodeAPI.dll";
#endif

	m_NvEncodeApiDll = LoadLibraryW(kNvEncodeApiDllName);

	if (m_NvEncodeApiDll == nullptr)
	{
		Utilities::Logging::Error("ERROR: Failed to load ", kNvEncodeApiDllName, ".");
		return;
	}

	typedef NVENCSTATUS (NVENCAPI* NvEncodeAPICreateInstanceFunc)(NV_ENCODE_API_FUNCTION_LIST *functionList);
	auto nvEncodeAPICreateInstance = reinterpret_cast<NvEncodeAPICreateInstanceFunc>(GetProcAddress(m_NvEncodeApiDll, "NvEncodeAPICreateInstance"));
	Assert(nvEncodeAPICreateInstance != nullptr);

	m_NvEncode.version = NV_ENCODE_API_FUNCTION_LIST_VER;

	auto nvstatus = nvEncodeAPICreateInstance(&m_NvEncode);
	Assert(nvstatus == NV_ENC_SUCCESS);
}

NvEncode::~NvEncode()
{
	if (m_NvEncodeApiDll != nullptr)
	{
		auto result = FreeLibrary(m_NvEncodeApiDll);
		Assert(result != FALSE);
	}
}
