#include "PrecompiledHeader.h"
#include "WindowStorage.h"

Utilities::CriticalSection Windowing::PreviewWindowStorage::s_HwndMapCriticalSection;
std::unordered_map<HWND, void*> Windowing::PreviewWindowStorage::s_HwndMap;