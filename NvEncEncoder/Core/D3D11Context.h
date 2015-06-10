#pragma once

namespace Utilities { class Logging; }

class D3D11Context
{
private:
	HMODULE m_D3D11Dll;
	Microsoft::WRL::ComPtr<ID3D11Device> m_Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_DeviceContext;

	static void PrintDeviceInfo(Utilities::Logging& logging, ID3D11Device* device, D3D_FEATURE_LEVEL featureLevel);

public:
	D3D11Context(Utilities::Logging& logging);
	~D3D11Context();

	D3D11Context(const D3D11Context&) = delete;
	D3D11Context& operator=(const D3D11Context&) = delete;

	inline ID3D11Device* GetDevice() { return m_Device.Get(); }
	inline ID3D11DeviceContext* GetDeviceContext() { return m_DeviceContext.Get(); }
};