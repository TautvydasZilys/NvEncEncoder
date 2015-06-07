#include "PrecompiledHeader.h"

HMODULE g_NvEncodeApiDll;
NV_ENCODE_API_FUNCTION_LIST g_NvAPI;

inline static void InitializeNvApiFunctions()
{
#if _WIN64
	const wchar_t nvEncodeApiDllName[] = L"nvEncodeAPI64.dll";
#else
	const wchar_t nvEncodeApiDllName[] = L"nvEncodeAPI.dll";
#endif

	g_NvEncodeApiDll = LoadLibraryW(nvEncodeApiDllName);
	Assert(g_NvEncodeApiDll != nullptr);

	typedef NVENCSTATUS(NVENCAPI* NvEncodeAPICreateInstanceFunc)(NV_ENCODE_API_FUNCTION_LIST *functionList);
	auto nvEncodeAPICreateInstance = reinterpret_cast<NvEncodeAPICreateInstanceFunc>(GetProcAddress(g_NvEncodeApiDll, "NvEncodeAPICreateInstance"));
	Assert(nvEncodeAPICreateInstance != nullptr);

	g_NvAPI.version = NV_ENCODE_API_FUNCTION_LIST_VER;

	auto nvstatus = nvEncodeAPICreateInstance(&g_NvAPI);
	Assert(nvstatus == NV_ENC_SUCCESS);
}

inline static void CleanupNvApiFunctions()
{
	ZeroStructure(&g_NvAPI);
	
	auto result = FreeLibrary(g_NvEncodeApiDll);
	Assert(result != FALSE);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	InitializeNvApiFunctions();

	CleanupNvApiFunctions();
	return 0;
}