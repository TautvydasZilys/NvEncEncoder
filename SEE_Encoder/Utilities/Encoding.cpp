#include "PrecompiledHeader.h"
#include "Encoding.h"

using namespace Utilities;

size_t Encoding::Utf8ToUtf16Inline(const char* str, size_t strLength, wchar_t* destination, size_t destinationLength)
{
	Assert(destinationLength >= strLength);
	Assert(strLength < static_cast<size_t>(std::numeric_limits<int>::max()));
	Assert(destinationLength < static_cast<size_t>(std::numeric_limits<int>::max()));

	auto length = MultiByteToWideChar(CP_UTF8, 0, str, static_cast<int>(strLength), destination, static_cast<int>(destinationLength - 1));
	Assert(length > 0);

	Assert(static_cast<size_t>(length) < destinationLength);
	destination[length] = '\0';

	return length;
}

std::wstring Encoding::Utf8ToUtf16(const char* str, size_t strLength)
{
	if (strLength == 0)
		return std::wstring();

	auto bufferSize = 2 * strLength;

	if (bufferSize < 2048)
	{
		wchar_t* buffer = static_cast<wchar_t*>(alloca(sizeof(wchar_t) * bufferSize));
		auto length = Utf8ToUtf16Inline(str, strLength, buffer, bufferSize);
		return std::wstring(buffer, length);
	}
	else
	{
		std::unique_ptr<wchar_t[]> buffer(new wchar_t[bufferSize]);
		auto length = Utf8ToUtf16Inline(str, strLength, buffer.get(), bufferSize);
		return std::wstring(buffer.get(), length);
	}
}

size_t Encoding::Utf16ToUtf8Inline(const wchar_t* wstr, size_t wstrLength, char* destination, size_t destinationLength)
{
	Assert(destinationLength >= wstrLength);
	Assert(wstrLength < static_cast<size_t>(std::numeric_limits<int>::max()));
	Assert(destinationLength < static_cast<size_t>(std::numeric_limits<int>::max()));

	auto length = WideCharToMultiByte(CP_UTF8, 0, wstr, static_cast<int>(wstrLength), destination, static_cast<int>(destinationLength - 1), nullptr, nullptr);
	Assert(length > 0);

	Assert(static_cast<size_t>(length) < destinationLength);
	destination[length] = '\0';

	return length;
}

std::string Encoding::Utf16ToUtf8(const wchar_t* wstr, size_t wstrLength)
{
	if (wstrLength == 0)
		return std::string();

	auto bufferSize = 4 * wstrLength;

	if (bufferSize < 4096)
	{
		char* buffer = static_cast<char*>(alloca(sizeof(char) * bufferSize));
		auto length = Utf16ToUtf8Inline(wstr, wstrLength, buffer, bufferSize);
		return std::string(buffer, length);
	}
	else
	{
		std::unique_ptr<char[]> buffer(new char[bufferSize]);
		auto length = Utf16ToUtf8Inline(wstr, wstrLength, buffer.get(), bufferSize);
		return std::string(buffer.get(), length);
	}
}