#pragma once

namespace Utilities
{
	namespace Encoding
	{
		template <size_t strLength>
		inline size_t Utf8ToUtf16Inline(const char (&str)[strLength], wchar_t* destination, size_t destinationLength);

		template <size_t destinationLength>
		inline size_t Utf8ToUtf16Inline(const char* str, size_t strLength, wchar_t (&destination)[destinationLength]);

		template <size_t destinationLength>
		inline size_t Utf8ToUtf16Inline(const std::string& str, wchar_t (&destination)[destinationLength]);

		template <size_t strLength, size_t destinationLength>
		inline size_t Utf8ToUtf16Inline(const char (&str)[strLength], wchar_t (&destination)[destinationLength]);

		inline size_t Utf8ToUtf16Inline(const std::string& str, wchar_t* destination, size_t destinationLength);
		size_t Utf8ToUtf16Inline(const char* str, size_t strLength, wchar_t* destination, size_t destinationLength);

		template <size_t strLength>
		inline std::wstring Utf8ToUtf16(const char (&str)[strLength]);
		inline std::wstring Utf8ToUtf16(const std::string& str);
		std::wstring Utf8ToUtf16(const char* str, size_t strLength);

		template <size_t wstrLength>
		inline size_t Utf16ToUtf8Inline(const wchar_t (&wstr)[wstrLength], char* destination, size_t destinationLength);

		template <size_t destinationLength>
		inline size_t Utf16ToUtf8Inline(const wchar_t* wstr, size_t wstrLength, char (&destination)[destinationLength]);

		template <size_t destinationLength>
		inline size_t Utf16ToUtf8Inline(const std::wstring wstr, size_t wstrLength, char (&destination)[destinationLength]);

		template <size_t wstrLength, size_t destinationLength>
		inline size_t Utf16ToUtf8Inline(const wchar_t (&wstr)[wstrLength], char (&destination)[destinationLength]);

		inline size_t Utf16ToUtf8Inline(const std::wstring& wstr, char* destination, size_t destinationLength);
		size_t Utf16ToUtf8Inline(const wchar_t* wstr, size_t wstrLength, char* destination, size_t destinationLength);

		template <size_t wstrLength>
		inline std::string Utf16ToUtf8(const wchar_t (&wstr)[wstrLength]);
		inline std::string Utf16ToUtf8(const std::wstring& wstr);
		std::string Utf16ToUtf8(const wchar_t* wstr, size_t wstrLength);
	}
}

#include "Encoding.inl"