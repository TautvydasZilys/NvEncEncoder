#pragma once

namespace Utilities { class Logging; }

class D3D11Context
{
private:
	HMODULE m_D3D11Dll;
	ID3D11Device* m_Device;
	ID3D11DeviceContext* m_DeviceContext;

	static void PrintDeviceInfo(Utilities::Logging& logging, ID3D11Device* device, D3D_FEATURE_LEVEL featureLevel);

public:
	D3D11Context(Utilities::Logging& logging);
	~D3D11Context();

	inline ID3D11Device* GetDevice() { return m_Device; }
	inline ID3D11DeviceContext* GetDeviceContext() { return m_DeviceContext; }
};