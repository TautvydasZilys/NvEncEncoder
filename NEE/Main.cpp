#include "PrecompiledHeader.h"
#include "NEE_Api.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	NEE_Initialize();

	auto encoder = NEE_CreateEncoder();

	if (encoder == nullptr)
		return -1;

	NEE_DestroyEncoder(encoder);
	NEE_Uninitialize();
	return 0;
}