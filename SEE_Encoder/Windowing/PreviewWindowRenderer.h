#pragma once

#include "Core\D3D11Context.h"
#include "Utilities\CriticalSection.h"
#include "Utilities\Event.h"

namespace Windowing
{
	class PreviewWindowRendererClass;

	class PreviewWindowRenderer
	{
	private:
		D3D11Context m_D3D11Context;
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_SwapChain;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_BackBufferRTV;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_SamplerState;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_ScaleBuffer;

		// Used only in DX11.1
		Microsoft::WRL::ComPtr<IDXGISwapChain2> m_SwapChain2;
		uint16_t m_SwapChainWidth;
		uint16_t m_SwapChainHeight;

		std::atomic<IDXGIResource*> m_OriginalSharedTexture;
		IDXGIResource* m_LastOriginalTexture;

		uint16_t m_TextureWidth;
		uint16_t m_TextureHeight;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_SourceTexture;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_StagingTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_StagingTextureSRV;

		uint32_t m_VertexBufferStride;
		uint32_t m_VertexBufferOffset;

		HWND m_Hwnd;
		uint16_t m_WindowWidth;
		uint16_t m_WindowHeight;

		bool m_WindowSizeDirty;

		bool& m_ShouldDestruct;
		Utilities::Event& m_DestroyedEvent;

		friend class ::Windowing::PreviewWindowRendererClass;

		void CreateOSWindow();
		void CreateD3D11Resources();

		void ResizeSwapchain();
		void UpdateScaleBuffer();
		bool RecreateStagingTexture();	// Returns whether size changed

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void Render();

	public:
		PreviewWindowRenderer(Utilities::Event& destroyedEvent, bool& shouldDestruct, Utilities::Logging& logging);
		~PreviewWindowRenderer();

		PreviewWindowRenderer(const PreviewWindowRenderer&) = delete;
		PreviewWindowRenderer& operator=(const PreviewWindowRenderer&) = delete;

		void WindowLoop();
		void Blit(ID3D11Texture2D* sharedTexture);

		void Destruct();
	};
}