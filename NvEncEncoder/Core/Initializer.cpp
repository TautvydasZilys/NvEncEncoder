#include "PrecompiledHeader.h"
#include "Initializer.h"
#include "Utilities\Logging.h"

Initializer::Initializer()
{
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	Utilities::Logging::Initialize();
}

Initializer::~Initializer()
{
	Utilities::Logging::Shutdown();
	CoUninitialize();
}
