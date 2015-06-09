#pragma once

#include "D3D11Context.h"
#include "Initializer.h"
#include "NvEncode.h"

class NEE_Encoder
{
private:
	Initializer m_Initializer;
	D3D11Context m_D3D11;
	NvEncode m_NvEncode;

public:
	inline NEE_Encoder() { }
	inline ~NEE_Encoder() { }

	inline bool IsValid() const { return m_D3D11.IsValid() && m_NvEncode.IsValid(); }
};

