#include "PrecompiledHeader.h"
#include "SEE_Api.h"
#include "SEE_Encoder.h"

SEE_Encoder* SEE_CreateEncoder(const wchar_t* logFileName, bool forceOverwriteLogFile)
{
	try
	{
		return new SEE_Encoder(logFileName, forceOverwriteLogFile);
	}
	catch (std::runtime_error&)
	{
	}

	return nullptr;
}

void SEE_DestroyEncoder(SEE_Encoder* encoder)
{
	delete encoder;
}