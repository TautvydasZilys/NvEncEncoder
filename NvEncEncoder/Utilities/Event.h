#pragma once

#include "CriticalSection.h"

namespace Utilities
{
	class Event
	{
	private:
		HANDLE m_Event;

#if _DEBUG
		CriticalSection m_CriticalSection;
		bool m_IsSet;
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
#if _DEBUG
			CriticalSection::Lock lock(m_CriticalSection);
			m_IsSet = true;
#endif
			auto result = SetEvent(m_Event);
			Assert(result != FALSE);
		}
		
		inline void Reset()
		{
#if _DEBUG
			CriticalSection::Lock lock(m_CriticalSection);
			m_IsSet = false;
#endif
			auto result = ResetEvent(m_Event);
			Assert(result != FALSE);
		}

		inline void Wait()
		{
#if _DEBUG
			CriticalSection::Lock lock(m_CriticalSection);

			if (!m_IsManualReset)
				m_IsSet = false;
#endif
			auto result = WaitForSingleObject(m_Event, INFINITE);
			Assert(result == WAIT_OBJECT_0);
		}
	};
}