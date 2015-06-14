#pragma once

#include "CriticalSection.h"

namespace Utilities
{
	class Event
	{
	private:
		HANDLE m_Event;

#if _DEBUG
		volatile bool m_IsSet;
		const bool m_IsManualReset;
#endif

	public:
		inline Event(bool isManualReset = false) :
			m_Event(CreateEventExW(nullptr, nullptr, isManualReset ? CREATE_EVENT_MANUAL_RESET : 0, EVENT_MODIFY_STATE | SYNCHRONIZE))
#if _DEBUG
			, m_IsSet(false), m_IsManualReset(isManualReset)
#endif
		{
		}

		inline ~Event()
		{
			auto result = CloseHandle(m_Event);
			Assert(result != FALSE);
		}

		Event(const Event&) = delete;
		Event& operator=(const Event&) = delete;

		inline void Set()
		{
			auto result = SetEvent(m_Event);
			Assert(result != FALSE);

#if _DEBUG
			m_IsSet = true;	// Not really thread safe, but whatever. It's there for debugging purposes only.
#endif
		}
		
		inline void Reset()
		{
			auto result = ResetEvent(m_Event);
			Assert(result != FALSE);

#if _DEBUG
			m_IsSet = false;
#endif
		}

		inline void Wait()
		{
			auto result = WaitForSingleObject(m_Event, INFINITE);
			Assert(result == WAIT_OBJECT_0);

#if _DEBUG
			if (!m_IsManualReset)
				m_IsSet = false;
#endif
		}
	};
}