#pragma once

class NvEncode
{
public:
	HMODULE m_NvEncodeApiDll;
	NV_ENCODE_API_FUNCTION_LIST m_NvEncode;

public:
	NvEncode();
	~NvEncode();

	inline bool IsValid() const { return m_NvEncodeApiDll != nullptr; }
	inline operator const NV_ENCODE_API_FUNCTION_LIST&() const { return m_NvEncode; }
};

