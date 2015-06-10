#pragma once

#include "Utilities\CriticalSection.h"
#include "Utilities\Event.h"

class D3D11Context;

class PreviewWindow
{
	struct
	{
		bool m_WindowSizeDirty;
		uint16_t m_WindowWidth;
		uint16_t m_WindowHeight;
		float m_TextureAspectRatio;
	} m_CrossThreadItems;	// These items must only be accessed when holding window critical section

	HWND m_Hwnd;
	bool m_IsDestroyed;
	Utilities::CriticalSection m_WindowCriticalSection;
	Utilities::Event m_DestroyedEvent;

	uint32_t m_VertexBufferStride;
	uint32_t m_VertexBufferOffset;

	Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_BackBufferRTV;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> m_SamplerState;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ScaleBuffer;

	static ATOM CreateWindowClass();
	void CreateOSWindow();
	void CreateD3D11Resources(D3D11Context& d3d11Context);
	void ResizeSwapchain(ID3D11DeviceContext* d3d11DeviceContext);
	void UpdateScaleBuffer(ID3D11DeviceContext* d3d11DeviceContext);

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void WindowLoop();
	void Cleanup();

public:
	PreviewWindow(D3D11Context& d3d11Context);
	~PreviewWindow();

	PreviewWindow(const PreviewWindow&) = delete;
	PreviewWindow& operator=(const PreviewWindow&) = delete;

	bool IsDestroyed() const { return m_IsDestroyed; }

	void Blit(ID3D11DeviceContext* d3d11DeviceContext, ID3D11ShaderResourceView* texture, int textureWidth, int textureHeight);
};

