#include "PrecompiledHeader.h"
#include "Logging.h"

Utilities::Logging::Logging(const wchar_t* logFileName, bool forceOverwrite)
{
	auto openMode = forceOverwrite ? CREATE_ALWAYS : CREATE_NEW;

	m_OutputFile = CreateFileW(logFileName, FILE_GENERIC_WRITE, FILE_SHARE_READ, nullptr, openMode, FILE_ATTRIBUTE_NORMAL, nullptr);
	Assert(m_OutputFile != INVALID_HANDLE_VALUE || !forceOverwrite);

	if (m_OutputFile == INVALID_HANDLE_VALUE)
	{
		Assert(GetLastError() == ERROR_FILE_EXISTS);

		m_OutputFile = CreateFileW(logFileName, FILE_GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		
		if (m_OutputFile == INVALID_HANDLE_VALUE)
			throw std::runtime_error("Failed to initialize logging.");

		SetFilePointer(m_OutputFile, 0, NULL, FILE_END);

		const char twoNewLines[] = "\r\n\r\n";
		DWORD bytesWritten;

		auto result = WriteFile(m_OutputFile, twoNewLines, sizeof(twoNewLines) - 1, &bytesWritten, nullptr);
		Assert(result != FALSE);
		Assert(bytesWritten == sizeof(twoNewLines));
	}
	else
	{
		const uint8_t utf8ByteOrderMark[] = { 0xEF, 0xBB, 0xBF };
		DWORD bytesWritten;

		auto result = WriteFile(m_OutputFile, utf8ByteOrderMark, sizeof(utf8ByteOrderMark), &bytesWritten, nullptr);
		Assert(result != FALSE);
		Assert(bytesWritten == sizeof(utf8ByteOrderMark));
	}

	SetLastError(ERROR_SUCCESS);

	Log("NvEnc Encoder initialized.");
}

Utilities::Logging::~Logging()
{
	Log("NvEnc Encoder is shutting down.");
	CloseHandle(m_OutputFile);
}

void Utilities::Logging::OutputMessage(const char* message, size_t length)
{
	if (IsDebuggerPresent())
		OutputDebugStringA(message);

	DWORD bytesWritten;

	auto result = WriteFile(m_OutputFile, message, static_cast<DWORD>(length), &bytesWritten, nullptr);
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
	delete this;
	__fastfail(errorCode); // Just crash� - let user know we crashed by bringing up WER dialog
}
