#pragma once

class NvEncode
{
private:
	HMODULE m_NvEncodeApiDll;
	NV_ENCODE_API_FUNCTION_LIST m_NvEncode;

public:
	NvEncode();
	~NvEncode();

	inline bool IsValid() const { return m_NvEncodeApiDll != nullptr; }
	inline const NV_ENCODE_API_FUNCTION_LIST* operator->() const { return &m_NvEncode; }
};

