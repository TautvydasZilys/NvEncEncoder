#pragma once

#if COMPILING_ENCODER
#define SEE_API extern "C" __declspec(dllexport)
#else
#define SEE_API extern "C" __declspec(dllimport)
#endif

class SEE_Encoder;

SEE_API SEE_Encoder* SEE_CreateEncoder(const wchar_t* logFileName, bool forceOverwriteLogFile);
SEE_API void SEE_DestroyEncoder(SEE_Encoder* encoder);