#pragma once

#if COMPILING_ENCODER
#define NEE_API extern "C" __declspec(dllexport)
#else
#define NEE_API extern "C" __declspec(dllimport)
#endif

class NEE_Encoder;

NEE_API NEE_Encoder* NEE_CreateEncoder(const wchar_t* logFileName, bool forceOverwriteLogFile);
NEE_API void NEE_DestroyEncoder(NEE_Encoder* encoder);