#include "PrecompiledHeader.h"
#include "PreviewWindowRenderer.h"
#include "Shaders\PreviewWindowPixelShader.h"
#include "Shaders\PreviewWindowVertexShader.h"
#include "WindowStorage.h"

using namespace Windowing;

namespace Windowing
{
	class PreviewWindowRendererClass
	{
	private:
		ATOM m_ClassAtom;

		inline PreviewWindowRendererClass()
		{
			WNDCLASSEXW windowInfo;

			ZeroStructure(&windowInfo);
			windowInfo.style = CS_CLASSDC | CS_HREDRAW | CS_VREDRAW;
			windowInfo.hInstance = GetModuleHandleW(nullptr);
			windowInfo.hCursor = LoadCursor(NULL, IDC_ARROW);
			windowInfo.lpszClassName = L"SEE_PreviewWindow";
			windowInfo.cbSize = sizeof(windowInfo);
			windowInfo.lpfnWndProc = PreviewWindowRenderer::WindowProc;

			m_ClassAtom = RegisterClassExW(&windowInfo);
		}

		inline ~PreviewWindowRendererClass()
		{
			UnregisterClassW(reinterpret_cast<LPCWSTR>(m_ClassAtom), GetModuleHandleW(nullptr));
		}

	public:
		inline static ATOM Get()
		{
			static PreviewWindowRendererClass s_Instance;
			return s_Instance.m_ClassAtom;
		}
	};
}

PreviewWindowRenderer::PreviewWindowRenderer(Utilities::Event& destroyedEvent, bool& shouldDestruct, Utilities::Logging& logging) :
	m_D3D11Context(logging),
	m_DestroyedEvent(destroyedEvent),
	m_ShouldDestruct(shouldDestruct),
	m_LastOriginalTexture(nullptr),
	m_TextureWidth(0),
	m_TextureHeight(0),
	m_WindowWidth(800),
	m_WindowHeight(450),
	m_WindowSizeDirty(false)
{
	CreateOSWindow();
	CreateD3D11Resources();
}

PreviewWindowRenderer::~PreviewWindowRenderer()
{
	if (m_LastOriginalTexture != nullptr)
		m_LastOriginalTexture->Release();

	DestroyWindow(m_Hwnd);
	PreviewWindowStorage::RemoveWindow(m_Hwnd);
	m_DestroyedEvent.Set();
}

void PreviewWindowRenderer::Destruct()
{
	m_ShouldDestruct = true;
}

void PreviewWindowRenderer::CreateOSWindow()
{
	const DWORD kWindowStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX | WS_VISIBLE;

	m_Hwnd = CreateWindowExW(WS_EX_APPWINDOW, reinterpret_cast<LPWSTR>(PreviewWindowRendererClass::Get()), L"SEE Preview", kWindowStyle,
		200, 200, m_WindowWidth, m_WindowHeight, nullptr, nullptr, GetModuleHandleW(nullptr), this);
	Assert(m_Hwnd != nullptr);
}

static inline void CreateSwapChain(HWND hwnd, ID3D11Device* d3d11Device, uint16_t width, uint16_t height, IDXGISwapChain*& swapChain, ID3D11RenderTargetView*& backBufferRTV)
{
	Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
	Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
	Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
	Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer;

	auto hr = d3d11Device->QueryInterface(dxgiDevice.ReleaseAndGetAddressOf());
	Assert(SUCCEEDED(hr));

	hr = dxgiDevice->GetAdapter(dxgiAdapter.ReleaseAndGetAddressOf());
	Assert(SUCCEEDED(hr));

	hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), &dxgiFactory);
	Assert(SUCCEEDED(hr));

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroStructure(&swapChainDesc);

	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

	hr = dxgiFactory->CreateSwapChain(d3d11Device, &swapChainDesc, &swapChain);
	
	if (FAILED(hr))
	{
		swapChainDesc.BufferCount = 1;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;	// DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL is supported only on Windows 8 and up

		hr = dxgiFactory->CreateSwapChain(d3d11Device, &swapChainDesc, &swapChain);
		Assert(SUCCEEDED(hr));
	}

	hr = swapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backBuffer);
	Assert(SUCCEEDED(hr));

	hr = d3d11Device->CreateRenderTargetView(backBuffer.Get(), nullptr, &backBufferRTV);
	Assert(SUCCEEDED(hr));
}

static inline void CreateShadersAndInputLayout(ID3D11Device* d3d11Device, ID3D11VertexShader*& vertexShader, ID3D11PixelShader*& pixelShader, ID3D11InputLayout*& inputLayout)
{
	D3D11_INPUT_ELEMENT_DESC inputDesc;
	ZeroStructure(&inputDesc);

	inputDesc.SemanticName = "POSITION";
	inputDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	inputDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	auto hr = d3d11Device->CreateInputLayout(&inputDesc, 1, g_PreviewWindowVertexShader, sizeof(g_PreviewWindowVertexShader), &inputLayout);
	Assert(SUCCEEDED(hr));

	hr = d3d11Device->CreateVertexShader(g_PreviewWindowVertexShader, sizeof(g_PreviewWindowVertexShader), nullptr, &vertexShader);
	Assert(SUCCEEDED(hr));

	hr = d3d11Device->CreatePixelShader(g_PreviewWindowPixelShader, sizeof(g_PreviewWindowPixelShader), nullptr, &pixelShader);
	Assert(SUCCEEDED(hr));
}

static inline void CreateBuffers(ID3D11Device* d3d11Device, ID3D11Buffer*& vertexBuffer, ID3D11Buffer*& scaleBuffer, uint32_t& vertexBufferStride, uint32_t& vertexBufferOffset)
{
	D3D11_BUFFER_DESC bufferDesc;
	ZeroStructure(&bufferDesc);

	struct Vertex { float x, y; };
	Vertex vertices[] =
	{
		{ -1.0f, -1.0f },
		{ -1.0f, 1.0f },
		{ 1.0f, -1.0f },
		{ 1.0f, 1.0f }
	};

	vertexBufferOffset = 0;

	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.StructureByteStride = vertexBufferStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA vertexData;
	ZeroStructure(&vertexData);
	vertexData.pSysMem = vertices;

	auto hr = d3d11Device->CreateBuffer(&bufferDesc, &vertexData, &vertexBuffer);
	Assert(SUCCEEDED(hr));

	ZeroStructure(&bufferDesc);
	bufferDesc.ByteWidth = 16;	// Even though our buffer only needs 8 bytes, d3d11 demands that cbuffers are at least 16 bytes wide.
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = d3d11Device->CreateBuffer(&bufferDesc, nullptr, &scaleBuffer);
	Assert(SUCCEEDED(hr));
}

static inline void CreateSamplerState(ID3D11Device* d3d11Device, ID3D11SamplerState*& samplerState)
{
	D3D11_SAMPLER_DESC samplerStateDesc;
	ZeroStructure(&samplerStateDesc);

	samplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	samplerStateDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerStateDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerStateDesc.MaxAnisotropy = 1;
	samplerStateDesc.BorderColor[0] = 0.0f;
	samplerStateDesc.BorderColor[1] = 0.0f;
	samplerStateDesc.BorderColor[2] = 0.0f;
	samplerStateDesc.BorderColor[3] = 0.0f;
	samplerStateDesc.MaxLOD = D3D11_FLOAT32_MAX;

	auto hr = d3d11Device->CreateSamplerState(&samplerStateDesc, &samplerState);
	Assert(SUCCEEDED(hr));
}

void PreviewWindowRenderer::CreateD3D11Resources()
{
	auto d3d11Device = m_D3D11Context.GetDevice();

	CreateSwapChain(m_Hwnd, d3d11Device, m_WindowWidth, m_WindowHeight, *m_SwapChain.ReleaseAndGetAddressOf(), *m_BackBufferRTV.ReleaseAndGetAddressOf());
	CreateShadersAndInputLayout(d3d11Device, *m_VertexShader.ReleaseAndGetAddressOf(), *m_PixelShader.ReleaseAndGetAddressOf(), *m_InputLayout.ReleaseAndGetAddressOf());
	CreateBuffers(d3d11Device, *m_VertexBuffer.ReleaseAndGetAddressOf(), *m_ScaleBuffer.ReleaseAndGetAddressOf(), m_VertexBufferStride, m_VertexBufferOffset);
	CreateSamplerState(d3d11Device, *m_SamplerState.ReleaseAndGetAddressOf());

	m_SwapChain.As(&m_SwapChain2);
	
	if (m_SwapChain2 != nullptr)
	{
		m_SwapChainWidth = m_WindowWidth;
		m_SwapChainHeight = m_WindowHeight;
	}
}

void PreviewWindowRenderer::ResizeSwapchain()
{
	HRESULT hr;

	if (m_SwapChain2 != nullptr)
	{
		if (m_WindowWidth <= m_SwapChainWidth && m_WindowHeight <= m_SwapChainHeight)
		{
			hr = m_SwapChain2->SetSourceSize(m_WindowWidth, m_WindowHeight);
			Assert(SUCCEEDED(hr));

			return;
		}
		else
		{
			m_SwapChainWidth = m_WindowWidth;
			m_SwapChainHeight = m_WindowHeight;
		}
	}


	Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer;

	m_D3D11Context.GetDeviceContext()->OMSetRenderTargets(0, nullptr, nullptr);
	m_BackBufferRTV = nullptr;

	hr = m_SwapChain->ResizeBuffers(0, m_WindowWidth, m_WindowHeight, DXGI_FORMAT_UNKNOWN, 0);
	Assert(SUCCEEDED(hr));

	hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backBuffer);
	Assert(SUCCEEDED(hr));

	hr = m_D3D11Context.GetDevice()->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_BackBufferRTV);
	Assert(SUCCEEDED(hr));
}

void PreviewWindowRenderer::UpdateScaleBuffer()
{
	struct { float x, y; } scale;

	auto windowAspectRatio = static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight);
	auto scaleBufferAspectRatio = static_cast<float>(m_TextureWidth) / static_cast<float>(m_TextureHeight);

	if (windowAspectRatio > scaleBufferAspectRatio)
	{
		// Vertical bars
		scale.x = scaleBufferAspectRatio / windowAspectRatio;
		scale.y = 1.0f;
	}
	else
	{
		// Horizontal bars
		scale.x = 1.0f;
		scale.y = windowAspectRatio / scaleBufferAspectRatio;
	}

	D3D11_MAPPED_SUBRESOURCE subresource;
	auto d3d11DeviceContext = m_D3D11Context.GetDeviceContext();

	auto hr = d3d11DeviceContext->Map(m_ScaleBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
	Assert(SUCCEEDED(hr));

	memcpy(subresource.pData, &scale, sizeof(scale));

	d3d11DeviceContext->Unmap(m_ScaleBuffer.Get(), 0);
}

bool PreviewWindowRenderer::RecreateStagingTexture()
{
	HANDLE sharedTextureHandle;
	auto d3d11Device = m_D3D11Context.GetDevice();
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;

	auto hr = m_LastOriginalTexture->GetSharedHandle(&sharedTextureHandle);
	Assert(SUCCEEDED(hr));
	Assert(sharedTextureHandle != nullptr);

	hr = d3d11Device->OpenSharedResource(sharedTextureHandle, __uuidof(ID3D11Resource), &resource);
	Assert(SUCCEEDED(hr));

	hr = resource.As(&m_SourceTexture);
	Assert(SUCCEEDED(hr));

	D3D11_TEXTURE2D_DESC textureDesc;
	m_SourceTexture->GetDesc(&textureDesc);

	if (textureDesc.Width == m_TextureWidth && textureDesc.Height == m_TextureHeight)
		return false;

	m_TextureWidth = static_cast<uint16_t>(textureDesc.Width);
	m_TextureHeight = static_cast<uint16_t>(textureDesc.Height);

	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	hr = d3d11Device->CreateTexture2D(&textureDesc, nullptr, &m_StagingTexture);
	Assert(SUCCEEDED(hr));

	hr = d3d11Device->CreateShaderResourceView(m_StagingTexture.Get(), nullptr, &m_StagingTextureSRV);
	Assert(SUCCEEDED(hr));

	return true;
}

LRESULT CALLBACK PreviewWindowRenderer::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CREATE:
		{
			auto createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
			auto windowInstance = static_cast<PreviewWindowRenderer*>(createStruct->lpCreateParams);
			PreviewWindowStorage::AddWindow(hwnd, windowInstance);
		}
		break;

		case WM_SIZE:
		{
			auto window = PreviewWindowStorage::RetrieveWindow<PreviewWindowRenderer>(hwnd);

			window->m_WindowWidth = LOWORD(lParam);
			window->m_WindowHeight = HIWORD(lParam);
			window->m_WindowSizeDirty = true;
			
			if (window->m_SwapChain != nullptr)
				window->Render();
		}
		break;

		case WM_CLOSE:
			PreviewWindowStorage::RetrieveWindow<PreviewWindowRenderer>(hwnd)->m_ShouldDestruct = true;
			return 0;
	}

	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void PreviewWindowRenderer::WindowLoop()
{
	MSG msg;

	while (!m_ShouldDestruct)
	{
		while (auto result = PeekMessageW(&msg, m_Hwnd, 0, 0, PM_REMOVE))
		{
			Assert(result != -1);

			if (result != FALSE)
				DispatchMessageW(&msg);
		}

		Render();
	}
}

void PreviewWindowRenderer::Render()
{
	bool textureSizeDirty = false;
	auto originalTexture = m_OriginalSharedTexture.exchange(nullptr);
	auto d3d11DeviceContext = m_D3D11Context.GetDeviceContext();

	if (originalTexture != nullptr)
	{
		if (originalTexture != m_LastOriginalTexture)
		{
			if (m_LastOriginalTexture != nullptr)
				m_LastOriginalTexture->Release();	// Release the old texture

			m_LastOriginalTexture = originalTexture;
			textureSizeDirty = RecreateStagingTexture();
		}
		else
		{
			originalTexture->Release();	// Just release it. We already have a reference to it
		}

		d3d11DeviceContext->CopyResource(m_StagingTexture.Get(), m_SourceTexture.Get());
	}

	if (m_WindowSizeDirty || textureSizeDirty)
		UpdateScaleBuffer();

	if (m_WindowSizeDirty)
	{
		if (m_WindowWidth == 0 || m_WindowHeight == 0)
			return;

		m_WindowSizeDirty = false;
		ResizeSwapchain();
	}

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	d3d11DeviceContext->ClearRenderTargetView(m_BackBufferRTV.Get(), clearColor);

	if (m_StagingTexture != nullptr)
	{
		const D3D11_VIEWPORT viewPort = { 0.0f, 0.0f, static_cast<float>(m_WindowWidth), static_cast<float>(m_WindowHeight), 0.0f, 1.0f };

		d3d11DeviceContext->IASetInputLayout(m_InputLayout.Get());
		d3d11DeviceContext->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &m_VertexBufferStride, &m_VertexBufferOffset);
		d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		d3d11DeviceContext->VSSetConstantBuffers(0, 1, m_ScaleBuffer.GetAddressOf());
		d3d11DeviceContext->VSSetShader(m_VertexShader.Get(), nullptr, 0);
		d3d11DeviceContext->RSSetViewports(1, &viewPort);
		d3d11DeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0);
		d3d11DeviceContext->PSSetSamplers(0, 1, m_SamplerState.GetAddressOf());
		d3d11DeviceContext->PSSetShaderResources(0, 1, m_StagingTextureSRV.GetAddressOf());
		d3d11DeviceContext->OMSetRenderTargets(1, m_BackBufferRTV.GetAddressOf(), nullptr);
		d3d11DeviceContext->Draw(4, 0);
	}

	m_SwapChain->Present(1, 0);
}

void PreviewWindowRenderer::Blit(ID3D11Texture2D* sharedTexture)
{
	IDXGIResource* dxgiResource;

	auto hr = sharedTexture->QueryInterface(&dxgiResource); // Render loop is responsible for releasing it
	Assert(SUCCEEDED(hr));

	auto other = m_OriginalSharedTexture.exchange(dxgiResource);

	if (other != nullptr)
	{
		// In this case, we ended up adding the new texture over the old one
		// before the render loop noticed it
		other->Release();
	}
}