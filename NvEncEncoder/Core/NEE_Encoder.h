#pragma once

#include "D3D11Context.h"
#include "NvEncode.h"

class NEE_Encoder
{
private:
	D3D11Context m_D3D11;
	NvEncode m_NvEncode;

public:
	inline NEE_Encoder() { }
	inline ~NEE_Encoder() { }

	inline bool IsValid() const { return m_D3D11.IsValid() && m_NvEncode.IsValid(); }
};

