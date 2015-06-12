#pragma once

#define COMPILING_ENCODER 1

#include <Windows.h>
#include <wrl.h>

#include <d3d11_2.h>

#include "External\nvEncodeAPI.h"

#undef min
#undef max

#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>

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