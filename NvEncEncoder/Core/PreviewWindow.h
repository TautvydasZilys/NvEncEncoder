#pragma once

#include "Utilities\CriticalSection.h"
#include "Utilities\Event.h"

class D3D11Context;

class PreviewWindow
{
	bool m_IsDestroyed;
	HWND m_Hwnd;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;

	Utilities::CriticalSection m_RenderCriticalSection;
	Utilities::Event m_DestroyedEvent;

	static ATOM CreateWindowClass();
	void CreateOSWindow();
	void CreateD3D11Resources(D3D11Context& d3d11Context);

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void WindowLoop();
	void Cleanup();

public:
	PreviewWindow(D3D11Context& d3d11Context);
	~PreviewWindow();

	PreviewWindow(const PreviewWindow&) = delete;
	PreviewWindow& operator=(const PreviewWindow&) = delete;

	bool IsDestroyed() const { return m_IsDestroyed; }
};

