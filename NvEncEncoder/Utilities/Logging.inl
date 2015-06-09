#include "Encoding.h"

inline void Utilities::Logging::Win32ErrorToMessageInline(int win32ErrorCode, wchar_t (&buffer)[kBufferSize])
{
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, win32ErrorCode, 0, buffer, kBufferSize, nullptr);
}

inline void Utilities::Logging::Win32ErrorToMessageInline(int win32ErrorCode, char (&buffer)[kBufferSize])
{
	wchar_t wBuffer[kBufferSize];

	auto messageLength = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, win32ErrorCode, 0, wBuffer, kBufferSize, nullptr);
	Utilities::Encoding::Utf16ToUtf8Inline(wBuffer, messageLength + 1, buffer, kBufferSize);
}

std::wstring Utilities::Logging::Win32ErrorToMessage(int win32ErrorCode)
{
	wchar_t buffer[kBufferSize];
	Win32ErrorToMessageInline(win32ErrorCode, buffer);
	return buffer;
}

template <typename Message>
inline void Utilities::Logging::OutputMessages(const Message& message)
{
	OutputMessage(message);
}

template <typename FirstMessage, typename ...Message>
inline void Utilities::Logging::OutputMessages(const FirstMessage& message, Message&& ...messages)
{
	OutputMessage(message);
	OutputMessages(std::forward<Message>(messages)...);
}

template <size_t Length>
inline void Utilities::Logging::OutputMessage(const wchar_t (&message)[Length])
{
	OutputMessage(message, Length - 1);
}

inline void Utilities::Logging::OutputMessage(const std::wstring& message)
{
	OutputMessage(message.c_str(), message.length());
}

inline void Utilities::Logging::OutputMessage(const wchar_t* message, size_t length)
{
	auto bufferSize = 4 * length;

	if (bufferSize < 4096)
	{
		char* buffer = static_cast<char*>(alloca(sizeof(char) * bufferSize));
		auto narrowLength = Encoding::Utf16ToUtf8Inline(message, length, buffer, bufferSize);
		OutputMessage(buffer, narrowLength);
	}
	else
	{
		std::unique_ptr<char[]> buffer(new char[bufferSize]);
		auto narrowLength = Encoding::Utf16ToUtf8Inline(message, length, buffer.get(), bufferSize);
		OutputMessage(buffer.get(), narrowLength);
	}
}

inline void Utilities::Logging::OutputMessage(size_t number)
{
	char buffer[20];
	auto length = sprintf_s(buffer, "%lld", number);
	OutputMessage(buffer, length);
}

template <size_t Length>
inline void Utilities::Logging::OutputMessage(const char (&message)[Length])
{
	OutputMessage(message, Length - 1);
}

inline void Utilities::Logging::OutputMessage(const std::string& message)
{
	OutputMessage(message.c_str(), message.length());
}

template<>
inline void Utilities::Logging::OutputMessage(const wchar_t* message, void* dummy)
{
	(void)dummy;
	OutputMessage(message, wcslen(message));
}

template<>
inline void Utilities::Logging::OutputMessage(const char* message, void* dummy)
{
	(void)dummy;
	OutputMessage(message, strlen(message));
}

template <typename ...Message>
inline void Utilities::Logging::Log(Message&& ...message)
{
	CriticalSection::Lock lock(m_LogCriticalSection);

	OutputCurrentTimestamp();
	OutputMessages(std::forward<Message>(message)...);
	OutputMessage("\r\n");
}

template <typename ...Message>
inline void Utilities::Logging::Error(Message&& ...message)
{
	Log(std::forward<Message>(message)...);
}

template <typename ...Message>
inline void Utilities::Logging::FatalError(Message&& ...message)
{
	Log("Terminating due to critical error:\r\n\t\t", std::forward<Message>(message)...);
	Terminate(-1);
}