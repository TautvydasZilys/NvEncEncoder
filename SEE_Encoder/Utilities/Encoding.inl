template <size_t strLength>
inline size_t Utilities::Encoding::Utf8ToUtf16Inline(const char (&str)[strLength], wchar_t* destination, size_t destinationLength)
{
	return Utf8ToUtf16Inline(str, strLength - 1, destination, destinationLength);
}

template <size_t destinationLength>
inline size_t Utilities::Encoding::Utf8ToUtf16Inline(const char* str, size_t strLength, wchar_t (&destination)[destinationLength])
{
	return Utf8ToUtf16Inline(str, strLength, destination, destinationLength);
}

template <size_t destinationLength>
inline size_t Utilities::Encoding::Utf8ToUtf16Inline(const std::string& str, wchar_t (&destination)[destinationLength])
{
	return Utf8ToUtf16Inline(str.c_str(), str.length(), destination, destinationLength);
}

template <size_t strLength, size_t destinationLength>
inline size_t Utilities::Encoding::Utf8ToUtf16Inline(const char (&str)[strLength], wchar_t (&destination)[destinationLength])
{
	return Utf8ToUtf16Inline(str, strLength - 1, destination, destinationLength);
}

inline size_t Utilities::Encoding::Utf8ToUtf16Inline(const std::string& str, wchar_t* destination, size_t destinationLength)
{
	return Utf8ToUtf16Inline(str.c_str(), str.length(), destination, destinationLength);
}

template <size_t strLength>
inline std::wstring Utilities::Encoding::Utf8ToUtf16(const char(&str)[strLength])
{
	return Utf8ToUtf16(str, strLength - 1);
}

inline std::wstring Utilities::Encoding::Utf8ToUtf16(const std::string& str)
{
	return Utf8ToUtf16(str.c_str(), str.length());
}

template <size_t wstrLength>
inline size_t Utilities::Encoding::Utf16ToUtf8Inline(const wchar_t (&wstr)[wstrLength], char* destination, size_t destinationLength)
{
	return Utf16ToUtf8Inline(wstr, wstrLength - 1, destination, destinationLength);
}

template <size_t destinationLength>
inline size_t Utilities::Encoding::Utf16ToUtf8Inline(const wchar_t* wstr, size_t wstrLength, char (&destination)[destinationLength])
{
	return Utf16ToUtf8Inline(wstr, wstrLength, destination, destinationLength);
}

template <size_t destinationLength>
inline size_t Utilities::Encoding::Utf16ToUtf8Inline(const std::wstring wstr, size_t wstrLength, char (&destination)[destinationLength])
{
	return Utf16ToUtf8Inline(wstr, wstrLength, destination, destinationLength);
}

template <size_t wstrLength, size_t destinationLength>
inline size_t Utilities::Encoding::Utf16ToUtf8Inline(const wchar_t (&wstr)[wstrLength], char (&destination)[destinationLength])
{
	return Utf16ToUtf8Inline(wstr, wstrLength - 1, destination, destinationLength);
}

inline size_t Utilities::Encoding::Utf16ToUtf8Inline(const std::wstring& wstr, char* destination, size_t destinationLength)
{
	return Utf16ToUtf8Inline(wstr.c_str(), wstr.length(), destination, destinationLength);
}

template <size_t wstrLength>
inline std::string Utf16ToUtf8(const wchar_t (&wstr)[wstrLength])
{
	return Utf16ToUtf8(wstr, wstrLength - 1);
}

inline std::string Utilities::Encoding::Utf16ToUtf8(const std::wstring& wstr)
{
	return Utf16ToUtf8(wstr.c_str(), wstr.length());
}