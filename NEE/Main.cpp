#include "PrecompiledHeader.h"
#include "NEE_Api.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	auto encoder = NEE_CreateEncoder(L"Encoder.log", false);

	if (encoder == nullptr)
		return -1;

	NEE_DestroyEncoder(encoder);
	return 0;
}