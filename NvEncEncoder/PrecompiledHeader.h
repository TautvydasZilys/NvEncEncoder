#pragma once

#include <Windows.h>

#include "External\nvEncodeAPI.h"

#if _DEBUG
#define Assert(x) do { if (!(x)) { if (IsDebuggerPresent()) { __debugbreak(); } } } while (false)
#else
#define Assert(x)
#endif

template <typename T>
inline void ZeroStructure(T* structure)
{
	ZeroMemory(structure, sizeof(T));
}