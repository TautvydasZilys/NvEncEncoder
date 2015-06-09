#pragma once

class D3D11Context;

class PreviewWindow
{
private:
	bool m_IsDestroyed;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;

	static ATOM CreateWindowClass();
	HWND CreateOSWindow();
	void CreateD3D11Resources(HWND hwnd, D3D11Context& d3d11Context);

	LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
	PreviewWindow(D3D11Context& d3d11Context);
	~PreviewWindow();

	bool IsDestroyed() const { return m_IsDestroyed; }
};

