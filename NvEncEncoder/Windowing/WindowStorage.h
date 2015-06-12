#pragma once

#include "Utilities\CriticalSection.h"

namespace Windowing
{
	class PreviewWindowStorage
	{
	private:
		static Utilities::CriticalSection s_HwndMapCriticalSection;
		static std::unordered_map<HWND, void*> s_HwndMap;

	public:
		PreviewWindowStorage() = delete;
		PreviewWindowStorage(const PreviewWindowStorage&) = delete;
		PreviewWindowStorage& operator=(const PreviewWindowStorage&) = delete;

		inline static void AddWindow(HWND hwnd, void* windowInstance)
		{
			Utilities::CriticalSection::Lock lock(s_HwndMapCriticalSection);
			s_HwndMap.insert(std::make_pair(hwnd, windowInstance));
		}

		template <typename T>
		inline static T* RetrieveWindow(HWND hwnd)
		{
			Utilities::CriticalSection::Lock lock(s_HwndMapCriticalSection);

			auto it = s_HwndMap.find(hwnd);
			Assert(it != s_HwndMap.end());

			return static_cast<T*>(it->second);
		}

		inline static void RemoveWindow(HWND hwnd)
		{
			Utilities::CriticalSection::Lock lock(s_HwndMapCriticalSection);
			s_HwndMap.erase(hwnd);
		}
	};
}