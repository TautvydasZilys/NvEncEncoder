#include "PrecompiledHeader.h"
#include "Logging.h"

Utilities::CriticalSection Utilities::Logging::s_LogCriticalSection;

static HANDLE s_OutputFile;
static const wchar_t kLogFileName[] = L"Encoder.log";

void Utilities::Logging::Initialize(bool forceOverwrite)
{
	auto openMode = forceOverwrite ? CREATE_ALWAYS : CREATE_NEW;

	s_OutputFile = CreateFileW(kLogFileName, FILE_GENERIC_WRITE, FILE_SHARE_READ, nullptr, openMode, FILE_ATTRIBUTE_NORMAL, nullptr);
	Assert(s_OutputFile != INVALID_HANDLE_VALUE || !forceOverwrite);

	if (s_OutputFile == INVALID_HANDLE_VALUE)
	{
		Assert(GetLastError() == ERROR_FILE_EXISTS);

		s_OutputFile = CreateFileW(kLogFileName, FILE_GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		Assert(s_OutputFile != INVALID_HANDLE_VALUE);

		SetFilePointer(s_OutputFile, 0, NULL, FILE_END);

		const char threeNewLines[] = "\r\n\r\n\r\n";
		DWORD bytesWritten;

		auto result = WriteFile(s_OutputFile, threeNewLines, sizeof(threeNewLines) - 1, &bytesWritten, nullptr);
		Assert(result != FALSE);
		Assert(bytesWritten = sizeof(threeNewLines));
	}
	else
	{
		const uint8_t utf8ByteOrderMark[] = { 0xEF, 0xBB, 0xBF };
		DWORD bytesWritten;

		auto result = WriteFile(s_OutputFile, utf8ByteOrderMark, sizeof(utf8ByteOrderMark), &bytesWritten, nullptr);
		Assert(result != FALSE);
		Assert(bytesWritten = sizeof(utf8ByteOrderMark));
	}

	SetLastError(ERROR_SUCCESS);

	Log("NvEnc Encoder launched.");
}

void Utilities::Logging::Shutdown()
{
	Log("NvEnc Encoder is shutting down.");
	CloseHandle(s_OutputFile);
}

void Utilities::Logging::OutputMessage(const char* message, size_t length)
{
	if (IsDebuggerPresent())
		OutputDebugStringA(message);

	DWORD bytesWritten;

	auto result = WriteFile(s_OutputFile, message, static_cast<DWORD>(length), &bytesWritten, nullptr);
	Assert(result != FALSE);
	Assert(bytesWritten == length);
}

static inline size_t SystemTimeToStringInline(wchar_t (&buffer)[Utilities::Logging::kBufferSize], SYSTEMTIME* systemTime = nullptr)
{
	auto dateLength = GetDateFormatEx(LOCALE_NAME_SYSTEM_DEFAULT, DATE_SHORTDATE, systemTime, nullptr, buffer, Utilities::Logging::kBufferSize, nullptr);
	buffer[dateLength - 1] = ' ';
	auto timeLength = GetTimeFormatEx(LOCALE_NAME_SYSTEM_DEFAULT, 0, systemTime, nullptr, buffer + dateLength, Utilities::Logging::kBufferSize - dateLength);

	return dateLength + timeLength;
}

static std::wstring SystemTimeToString(SYSTEMTIME* systemTime = nullptr)
{
	wchar_t buffer[Utilities::Logging::kBufferSize];
	SystemTimeToStringInline(buffer, systemTime);
	return buffer;
}

void Utilities::Logging::OutputCurrentTimestamp()
{
	OutputMessage("[");

	char buffer[kBufferSize];
	wchar_t wbuffer[kBufferSize];
	auto dateTimeLength = SystemTimeToStringInline(wbuffer);
	auto strLength = Encoding::Utf16ToUtf8Inline(wbuffer, dateTimeLength - 1, buffer, kBufferSize);

	OutputMessage(buffer, strLength);
	OutputMessage("] ");
}

void Utilities::Logging::Terminate(int errorCode)
{
	Logging::Shutdown();
	__fastfail(errorCode); // Just crash™ - let user know we crashed by bringing up WER dialog
}
