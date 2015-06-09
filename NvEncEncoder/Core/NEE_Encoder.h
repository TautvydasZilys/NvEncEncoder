#pragma once

#include "D3D11Context.h"
#include "NvEncode.h"
#include "Utilities\Logging.h"

class NEE_Encoder
{
private:
	Utilities::Logging m_Logging;
	D3D11Context m_D3D11;
	NvEncode m_NvEncode;

public:
	inline NEE_Encoder(const wchar_t* logFileName, bool forceOverwriteLogFile) :
		m_Logging(logFileName, forceOverwriteLogFile),
		m_D3D11(m_Logging),
		m_NvEncode(m_Logging)
	{
	}

	inline ~NEE_Encoder() { }
};

