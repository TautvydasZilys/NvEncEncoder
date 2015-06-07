#include "PrecompiledHeader.h"
#include "Initializer.h"
#include "D3D11Context.h"
#include "NvEncode.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	Initializer initializer;
	D3D11Context d3d11;

	if (!d3d11.IsValid())
		return -1;

	NvEncode nvEncode;

	if (!nvEncode.IsValid())
		return -1;

	return 0;
}