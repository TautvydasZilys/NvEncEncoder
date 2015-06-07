#include "PrecompiledHeader.h"

HMODULE g_NvEncodeApiDll;
HMODULE g_D3D11Dll;

NV_ENCODE_API_FUNCTION_LIST g_NvAPI;
ID3D11Device* g_D3D11Device;
ID3D11DeviceContext* g_D3D11DeviceContext;

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

inline static void InitializeD3D11()
{
	g_D3D11Dll = LoadLibraryW(L"d3d11.dll");
	Assert(g_D3D11Dll != nullptr);

	typedef HRESULT (WINAPI* D3D11CreateDeviceFunc)(
		_In_opt_ IDXGIAdapter* pAdapter,
		D3D_DRIVER_TYPE DriverType,
		HMODULE Software,
		UINT Flags,
		_In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels,
		UINT FeatureLevels,
		UINT SDKVersion,
		_Out_opt_ ID3D11Device** ppDevice,
		_Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel,
		_Out_opt_ ID3D11DeviceContext** ppImmediateContext);

	auto d3d11CreateDevice = reinterpret_cast<D3D11CreateDeviceFunc>(GetProcAddress(g_D3D11Dll, "D3D11CreateDevice"));
	Assert(d3d11CreateDevice != nullptr);

	D3D_FEATURE_LEVEL featureLevel;
	D3D_FEATURE_LEVEL featureLevels[] = 
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};

	UINT d3d11Flags = 0;

#if _DEBUG
	d3d11Flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	auto hr = d3d11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, d3d11Flags, featureLevels,
		ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &g_D3D11Device, &featureLevel, &g_D3D11DeviceContext);
	Assert(SUCCEEDED(hr));
}

inline static void CleanupD3D11()
{
	g_D3D11DeviceContext->Release();
	g_D3D11Device->Release();
	
	auto result = FreeLibrary(g_D3D11Dll);
	Assert(result != FALSE);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	InitializeNvApiFunctions();
	InitializeD3D11();

	CleanupD3D11();
	CleanupNvApiFunctions();
	return 0;
}