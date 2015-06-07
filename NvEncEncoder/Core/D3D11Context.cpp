#include "PrecompiledHeader.h"
#include "D3D11Context.h"
#include "Utilities\Logging.h"

D3D11Context::D3D11Context() :
	m_D3D11Dll(nullptr),
	m_Device(nullptr),
	m_DeviceContext(nullptr)
{
	m_D3D11Dll = LoadLibraryW(L"d3d11.dll");

	if (m_D3D11Dll == nullptr)
	{
		Utilities::Logging::Error("ERROR: Failed to load d3d11.dll.");
		return;
	}

	typedef HRESULT(WINAPI* D3D11CreateDeviceFunc)(
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

	auto d3d11CreateDevice = reinterpret_cast<D3D11CreateDeviceFunc>(GetProcAddress(m_D3D11Dll, "D3D11CreateDevice"));
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
		ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &m_Device, &featureLevel, &m_DeviceContext);
	Assert(SUCCEEDED(hr));

	PrintDeviceInfo(m_Device, featureLevel);
}

D3D11Context::~D3D11Context()
{
	if (m_D3D11Dll != nullptr)
	{
		Assert(m_Device != nullptr);
		Assert(m_DeviceContext != nullptr);

		m_DeviceContext->Release();
		m_Device->Release();

		auto result = FreeLibrary(m_D3D11Dll);
		Assert(result != FALSE);
	}
}

void D3D11Context::PrintDeviceInfo(ID3D11Device* device, D3D_FEATURE_LEVEL featureLevel)
{
	IDXGIDevice* dxgiDevice;
	IDXGIAdapter* dxgiAdapter;
	DXGI_ADAPTER_DESC dxgiAdapterDesc;

	auto hr = device->QueryInterface(&dxgiDevice);
	Assert(SUCCEEDED(hr));

	hr = dxgiDevice->GetAdapter(&dxgiAdapter);
	Assert(SUCCEEDED(hr));

	hr = dxgiAdapter->GetDesc(&dxgiAdapterDesc);
	Assert(SUCCEEDED(hr));

	const char* featureLevelString = "Unknown";

	switch (featureLevel)
	{
	case D3D_FEATURE_LEVEL_11_1:
		featureLevelString = "11.1";
		break;

	case D3D_FEATURE_LEVEL_11_0:
		featureLevelString = "11.0";
		break;

	case D3D_FEATURE_LEVEL_10_1:
		featureLevelString = "10.1";
		break;

	case D3D_FEATURE_LEVEL_10_0:
		featureLevelString = "10.0";
		break;
	}

	const auto indentation = "    ";
	Utilities::Logging::Log("Created Direct3D 11 device.");
	Utilities::Logging::Log(indentation, "Used GPU name: ", dxgiAdapterDesc.Description, ".");
	Utilities::Logging::Log(indentation, "Dedicated video memory: ", dxgiAdapterDesc.DedicatedVideoMemory / 1024 / 1024, " MB.");
	Utilities::Logging::Log(indentation, "Dedicated system memory: ", dxgiAdapterDesc.DedicatedSystemMemory / 1024 / 1024, " MB.");
	Utilities::Logging::Log(indentation, "Shared system memory: ", dxgiAdapterDesc.SharedSystemMemory / 1024 / 1024, " MB.");
	Utilities::Logging::Log(indentation, "Feature level: ", featureLevelString, ".");

	dxgiAdapter->Release();
	dxgiDevice->Release();
}