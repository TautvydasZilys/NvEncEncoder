#include "PrecompiledHeader.h"
#include "SEE_Api.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	UNREFERENCED(hInstance);
	UNREFERENCED(hPrevInstance);
	UNREFERENCED(pCmdLine);
	UNREFERENCED(nCmdShow);

	auto encoder = SEE_CreateEncoder(L"Encoder.log", false);

	if (encoder == nullptr)
		return -1;

	SEE_DestroyEncoder(encoder);
	return 0;
}