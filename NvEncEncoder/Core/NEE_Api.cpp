#include "PrecompiledHeader.h"
#include "NEE_Api.h"
#include "NEE_Encoder.h"

NEE_Encoder* NEE_CreateEncoder()
{
	auto encoder = new NEE_Encoder();

	if (!encoder->IsValid())
	{
		delete encoder;
		return nullptr;
	}

	return encoder;
}

void NEE_DestroyEncoder(NEE_Encoder* encoder)
{
	delete encoder;
}