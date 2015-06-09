#pragma once

#include "CriticalSection.h"

namespace Utilities
{
	class Logging
	{
	public:
		static const int kBufferSize = 256;
		inline void Win32ErrorToMessageInline(int win32ErrorCode, wchar_t (&buffer)[kBufferSize]);
		inline void Win32ErrorToMessageInline(int win32ErrorCode, char (&buffer)[kBufferSize]);
		inline std::wstring Win32ErrorToMessage(int win32ErrorCode);

		template <typename ...Message>
		inline void Log(Message&& ...message);
		template <typename ...Message>
		inline void Error(Message&& ...message);
		template <typename ...Message>
		inline void FatalError(Message&& ...message);
		
		Logging(const wchar_t* logFileName, bool forceOverwrite);
		~Logging();

		Logging(const Logging&) = delete;
		Logging& operator=(const Logging&) = delete;

	private:
		// Output* functions are NOT thread safe
		template <size_t Length>
		inline void OutputMessage(const wchar_t (&message)[Length]);
		inline void OutputMessage(const std::wstring& message);
		inline void OutputMessage(const wchar_t* message, size_t length);

		inline void OutputMessage(size_t number);

		template <size_t Length>
		inline void OutputMessage(const char(&message)[Length]);
		inline void OutputMessage(const std::string& message);

		// Some template magic to make precendence of char* overload lower than the char[n]
		template <typename T>
		inline void OutputMessage(T message, void* dummy = nullptr) { static_assert(false, "Missing template specialization for OutputMessage."); }
		template <>
		inline void OutputMessage<const char*>(const char* message, void* dummy);
		template <>
		inline void OutputMessage<const wchar_t*>(const wchar_t* message, void* dummy);

		void OutputMessage(const char* message, size_t length);
		void OutputCurrentTimestamp();

		template <typename Message>
		inline void OutputMessages(const Message& message);

		template <typename FirstMessage, typename ...Message>
		inline void OutputMessages(const FirstMessage& message, Message&& ...messages);

		void Terminate(int errorCode = -1);

		HANDLE m_OutputFile;
		CriticalSection m_LogCriticalSection;
	};
}

#include "Logging.inl"