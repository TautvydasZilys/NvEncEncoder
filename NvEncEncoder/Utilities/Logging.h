#pragma once

#include "CriticalSection.h"

namespace Utilities
{
	class Logging
	{
	public:
		static const int kBufferSize = 256;
		static inline void Win32ErrorToMessageInline(int win32ErrorCode, wchar_t (&buffer)[kBufferSize]);
		static inline void Win32ErrorToMessageInline(int win32ErrorCode, char (&buffer)[kBufferSize]);
		static inline std::wstring Win32ErrorToMessage(int win32ErrorCode);

		template <typename ...Message>
		static inline void Log(Message&& ...message);
		template <typename ...Message>
		static inline void Error(Message&& ...message);
		template <typename ...Message>
		static inline void FatalError(Message&& ...message);

		static void Initialize(bool forceOverwrite = false);
		static void Shutdown();

		Logging() = delete;
		Logging(const Logging&) = delete;
		~Logging() = delete;

	private:
		// Output* functions are NOT thread safe
		template <size_t Length>
		static inline void OutputMessage(const wchar_t (&message)[Length]);
		static inline void OutputMessage(const std::wstring& message);
		static inline void OutputMessage(const wchar_t* message, size_t length);

		static inline void OutputMessage(size_t number);

		template <size_t Length>
		static inline void OutputMessage(const char(&message)[Length]);
		static inline void OutputMessage(const std::string& message);

		// Some template magic to make precendence of char* overload lower than the char[n]
		template <typename T>
		static inline void OutputMessage(T message, void* dummy = nullptr) { static_assert(false, "Missing template specialization for OutputMessage."); }
		template <>
		static inline void OutputMessage<const char*>(const char* message, void* dummy);
		template <>
		static inline void OutputMessage<const wchar_t*>(const wchar_t* message, void* dummy);

		static void OutputMessage(const char* message, size_t length);
		static void OutputCurrentTimestamp();

		template <typename Message>
		static inline void OutputMessages(const Message& message);

		template <typename FirstMessage, typename ...Message>
		static inline void OutputMessages(const FirstMessage& message, Message&& ...messages);

		static void Terminate(int errorCode = -1);

		static CriticalSection s_LogCriticalSection;
	};
}

#include "Logging.inl"