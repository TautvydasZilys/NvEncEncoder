#pragma once

namespace Utilities { class Logging; }

namespace Encoding
{
	class NvEncode
	{
	private:
		HMODULE m_NvEncodeApiDll;
		NV_ENCODE_API_FUNCTION_LIST m_NvEncode;

	public:
		NvEncode(Utilities::Logging& logging);
		~NvEncode();

		inline const NV_ENCODE_API_FUNCTION_LIST* operator->() const { return &m_NvEncode; }
	};
}