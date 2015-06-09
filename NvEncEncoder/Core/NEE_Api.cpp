#include "PrecompiledHeader.h"
#include "Initializer.h"
#include "NEE_Api.h"
#include "NEE_Encoder.h"

static Initializer* g_Initializer;

bool NEE_Initialize()
{
	if (g_Initializer != nullptr)
		return false;

	g_Initializer = new Initializer;
	return true;
}

bool NEE_Uninitialize()
{
	if (g_Initializer == nullptr)
		return false;

	delete g_Initializer;
	return true;
}

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