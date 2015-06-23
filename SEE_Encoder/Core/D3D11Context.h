#pragma once

#include "Utilities\Com\ComPtr.h"

namespace Utilities { class Logging; }

class D3D11Context
{
private:
	HMODULE m_D3D11Dll;
	Utilities::Com::ComPtr<ID3D11Device> m_Device;
	Utilities::Com::ComPtr<ID3D11DeviceContext> m_DeviceContext;
	D3D_FEATURE_LEVEL m_FeatureLevel;

#if _DEBUG
	const DWORD m_ThreadId;
#endif

public:
	D3D11Context(Utilities::Logging& logging);
	~D3D11Context();

	D3D11Context(const D3D11Context&) = delete;
	D3D11Context& operator=(const D3D11Context&) = delete;

	inline ID3D11Device* GetDevice()
	{
#if _DEBUG
		Assert(GetCurrentThreadId() == m_ThreadId);
#endif
		return m_Device;
	}

	inline ID3D11DeviceContext* GetDeviceContext()
	{
#if _DEBUG
		Assert(GetCurrentThreadId() == m_ThreadId);
#endif
		return m_DeviceContext;
	}

	void PrintDeviceInfo(Utilities::Logging& logging);
};