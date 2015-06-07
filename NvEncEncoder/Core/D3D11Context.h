#pragma once

class D3D11Context
{
private:
	HMODULE m_D3D11Dll;
	ID3D11Device* m_Device;
	ID3D11DeviceContext* m_DeviceContext;

	static void PrintDeviceInfo(ID3D11Device* device, D3D_FEATURE_LEVEL featureLevel);

public:
	D3D11Context();
	~D3D11Context();

	inline bool IsValid() const { return m_Device != nullptr; }

	inline ID3D11Device* GetDevice() { Assert(m_Device != nullptr); return m_Device; }
	inline ID3D11DeviceContext* GetDeviceContext() { Assert(m_DeviceContext != nullptr); return m_DeviceContext; }
};