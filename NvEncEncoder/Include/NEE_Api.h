#pragma once

#if COMPILING_ENCODER
#define NEE_API extern "C" __declspec(dllexport)
#else
#define NEE_API extern "C" __declspec(dllimport)
#endif

class NEE_Encoder;

NEE_API bool NEE_Initialize();
NEE_API bool NEE_Uninitialize();

NEE_API NEE_Encoder* NEE_CreateEncoder();
NEE_API void NEE_DestroyEncoder(NEE_Encoder* encoder);