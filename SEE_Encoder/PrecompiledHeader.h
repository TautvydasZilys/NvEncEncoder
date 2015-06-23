#pragma once

#define COMPILING_ENCODER 1

#define WINVER _WIN32_WINNT_VISTA
#define _WIN32_WINNT _WIN32_WINNT_VISTA
#define NTDDI_VERSION NTDDI_VISTA

#include <evr.h>
#include <mfapi.h>
#include <Mferror.h>
#include <Mfidl.h>
#include <Windows.h>
#include <wrl.h>

#include <d3d9.h>
#include <d3d11_2.h>

#include "External\nvEncodeAPI.h"

#undef min
#undef max

#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>

#define UNREFERENCED(x) do { if (0, 0) { (void)(x); } } while (0, 0)

#if _DEBUG
#define Assert(x) do { if (!(0,(x))) { if (IsDebuggerPresent()) { __debugbreak(); } } } while (0, 0)
#else
#define Assert(x) UNREFERENCED(x)
#endif

template <typename T>
inline void ZeroStructure(T* structure)
{
	ZeroMemory(structure, sizeof(T));
}