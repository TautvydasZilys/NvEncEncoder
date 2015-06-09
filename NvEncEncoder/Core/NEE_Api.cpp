#include "PrecompiledHeader.h"
#include "NEE_Api.h"
#include "NEE_Encoder.h"

NEE_Encoder* NEE_CreateEncoder(const wchar_t* logFileName, bool forceOverwriteLogFile)
{
	try
	{
		return new NEE_Encoder(logFileName, forceOverwriteLogFile);
	}
	catch (std::runtime_error&)
	{
	}

	return nullptr;
}

void NEE_DestroyEncoder(NEE_Encoder* encoder)
{
	delete encoder;
}