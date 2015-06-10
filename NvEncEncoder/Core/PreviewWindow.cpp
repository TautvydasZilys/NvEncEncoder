#include "PrecompiledHeader.h"
#include "D3D11Context.h"
#include "PreviewWindow.h"
#include "Shaders\PreviewWindowPixelShader.h"
#include "Shaders\PreviewWindowVertexShader.h"

class PreviewWindowStorage
{
private:
	static Utilities::CriticalSection s_HwndMapCriticalSection;
	static std::unordered_map<HWND, PreviewWindow*> s_HwndMap;
	
public:
	PreviewWindowStorage() = delete;
	PreviewWindowStorage(const PreviewWindowStorage&) = delete;
	PreviewWindowStorage& operator=(const PreviewWindowStorage&) = delete;

	inline static void AddWindow(HWND hwnd, PreviewWindow* windowInstance)
	{
		Utilities::CriticalSection::Lock lock(s_HwndMapCriticalSection);
		s_HwndMap.insert(std::make_pair(hwnd, windowInstance));
	}

	inline static PreviewWindow* RetrieveWindow(HWND hwnd)
	{
		Utilities::CriticalSection::Lock lock(s_HwndMapCriticalSection);
		
		auto it = s_HwndMap.find(hwnd);
		Assert(it != s_HwndMap.end());

		return it->second;
	}

	inline static void RemoveWindow(HWND hwnd)
	{
		Utilities::CriticalSection::Lock lock(s_HwndMapCriticalSection);
		s_HwndMap.erase(hwnd);
	}
};

Utilities::CriticalSection PreviewWindowStorage::s_HwndMapCriticalSection;
std::unordered_map<HWND, PreviewWindow*> PreviewWindowStorage::s_HwndMap;

struct WindowThreadContext
{
	PreviewWindow& previewWindow;
	Utilities::Event& windowCreationEvent;

	inline WindowThreadContext(PreviewWindow& previewWindow, Utilities::Event& windowCreationEvent) :
		previewWindow(previewWindow), windowCreationEvent(windowCreationEvent)
	{
	}
};

PreviewWindow::PreviewWindow(D3D11Context& d3d11Context) :
	m_IsDestroyed(false), m_DestroyedEvent(true)
{
	ZeroStructure(&m_CrossThreadItems);

	m_CrossThreadItems.m_WindowWidth = 800;
	m_CrossThreadItems.m_WindowHeight = 450;	

	Utilities::Event windowCreationEvent;
	WindowThreadContext windowThreadContext(*this, windowCreationEvent);

	auto windowThreadEntry = [](LPVOID lpThreadParameter) -> DWORD
	{
		WindowThreadContext& context = *static_cast<WindowThreadContext*>(lpThreadParameter);
		auto& previewWindow = context.previewWindow;

		previewWindow.CreateOSWindow();

		// NOTE: after this event is set, can't reference context anymore!
		context.windowCreationEvent.Set();

		previewWindow.WindowLoop();
		previewWindow.Cleanup();

		return 0;
	};

	auto threadHandle = CreateThread(nullptr, 64 * 1024, windowThreadEntry, &windowThreadContext, 0, nullptr);
	Assert(threadHandle != nullptr);

	auto result = CloseHandle(threadHandle);
	Assert(result != FALSE);

	windowCreationEvent.Wait();
	CreateD3D11Resources(d3d11Context);
}

PreviewWindow::~PreviewWindow()
{
	if (!m_IsDestroyed)
		PostMessageW(m_Hwnd, WM_CLOSE, 0, 0);

	m_DestroyedEvent.Wait();
	PreviewWindowStorage::RemoveWindow(m_Hwnd);
}

void PreviewWindow::Cleanup()
{
	Assert(m_IsDestroyed);

	DestroyWindow(m_Hwnd);
	m_DestroyedEvent.Set();
}

ATOM PreviewWindow::CreateWindowClass()
{
	WNDCLASSEXW windowInfo;

	ZeroStructure(&windowInfo);
	windowInfo.style = CS_CLASSDC | CS_HREDRAW | CS_VREDRAW;
	windowInfo.hInstance = GetModuleHandleW(nullptr);
	windowInfo.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowInfo.lpszClassName = L"NEE_PreviewWindow";
	windowInfo.cbSize = sizeof(windowInfo);
	windowInfo.lpfnWndProc = WindowProc;

	auto classAtom = RegisterClassExW(&windowInfo);
	Assert(classAtom != 0);
	return classAtom;
}

void PreviewWindow::CreateOSWindow()
{
	static ATOM s_WindowClassAtom = CreateWindowClass();
	const DWORD kWindowStyle = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX | WS_VISIBLE;

	Utilities::CriticalSection::Lock lock(m_WindowCriticalSection);

	m_Hwnd = CreateWindowExW(WS_EX_APPWINDOW, reinterpret_cast<LPWSTR>(s_WindowClassAtom), L"NEE Preview", kWindowStyle,
		200, 200, m_CrossThreadItems.m_WindowWidth, m_CrossThreadItems.m_WindowHeight, nullptr, nullptr, GetModuleHandleW(nullptr), this);
	Assert(m_Hwnd != nullptr);
}

static inline void CreateSwapChain(HWND hwnd, ID3D11Device* d3d11Device, uint16_t width, uint16_t height, IDXGISwapChain** swapChain, ID3D11RenderTargetView** backBufferRTV)
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
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hwnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	hr = dxgiFactory->CreateSwapChain(d3d11Device, &swapChainDesc, swapChain);
	Assert(SUCCEEDED(hr));

	hr = (*swapChain)->GetBuffer(0, __uuidof(ID3D11Resource), &backBuffer);
	Assert(SUCCEEDED(hr));

	hr = d3d11Device->CreateRenderTargetView(backBuffer.Get(), nullptr, backBufferRTV);
	Assert(SUCCEEDED(hr));
}

static inline void CreateShadersAndInputLayout(ID3D11Device* d3d11Device, ID3D11VertexShader** vertexShader, ID3D11PixelShader** pixelShader, ID3D11InputLayout** inputLayout)
{
	D3D11_INPUT_ELEMENT_DESC inputDesc;
	ZeroStructure(&inputDesc);

	inputDesc.SemanticName = "POSITION";
	inputDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	inputDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	auto hr = d3d11Device->CreateInputLayout(&inputDesc, 1, g_PreviewWindowVertexShader, sizeof(g_PreviewWindowVertexShader), inputLayout);
	Assert(SUCCEEDED(hr));

	hr = d3d11Device->CreateVertexShader(g_PreviewWindowVertexShader, sizeof(g_PreviewWindowVertexShader), nullptr, vertexShader);
	Assert(SUCCEEDED(hr));

	hr = d3d11Device->CreatePixelShader(g_PreviewWindowPixelShader, sizeof(g_PreviewWindowPixelShader), nullptr, pixelShader);
	Assert(SUCCEEDED(hr));
}

static inline void CreateBuffers(ID3D11Device* d3d11Device, ID3D11Buffer** vertexBuffer, ID3D11Buffer** scaleBuffer, uint32_t* vertexBufferStride, uint32_t* vertexBufferOffset)
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

	*vertexBufferOffset = 0;

	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.StructureByteStride = *vertexBufferStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA vertexData;
	ZeroStructure(&vertexData);
	vertexData.pSysMem = vertices;

	auto hr = d3d11Device->CreateBuffer(&bufferDesc, &vertexData, vertexBuffer);
	Assert(SUCCEEDED(hr));

	ZeroStructure(&bufferDesc);
	bufferDesc.ByteWidth = 16;	// Even though our buffer only needs 8 bytes, d3d11 demands that cbuffers are at least 16 bytes wide.
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = d3d11Device->CreateBuffer(&bufferDesc, nullptr, scaleBuffer);
	Assert(SUCCEEDED(hr));
}

static inline void CreateSamplerState(ID3D11Device* d3d11Device, ID3D11SamplerState** samplerState)
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

	auto hr = d3d11Device->CreateSamplerState(&samplerStateDesc, samplerState);
	Assert(SUCCEEDED(hr));
}

void PreviewWindow::CreateD3D11Resources(D3D11Context& d3d11Context)
{
	uint16_t windowWidth, windowHeight;

	{
		Utilities::CriticalSection::Lock lock(m_WindowCriticalSection);
		windowWidth = m_CrossThreadItems.m_WindowWidth;
		windowHeight = m_CrossThreadItems.m_WindowHeight;
	}

	auto d3d11Device = d3d11Context.GetDevice();

	CreateSwapChain(m_Hwnd, d3d11Device, windowWidth, windowHeight, m_SwapChain.ReleaseAndGetAddressOf(), m_BackBufferRTV.ReleaseAndGetAddressOf());
	CreateShadersAndInputLayout(d3d11Device, m_VertexShader.ReleaseAndGetAddressOf(), m_PixelShader.ReleaseAndGetAddressOf(), m_InputLayout.ReleaseAndGetAddressOf());
	CreateBuffers(d3d11Device, m_VertexBuffer.ReleaseAndGetAddressOf(), m_ScaleBuffer.ReleaseAndGetAddressOf(), &m_VertexBufferStride, &m_VertexBufferOffset);
	CreateSamplerState(d3d11Device, m_SamplerState.ReleaseAndGetAddressOf());
}

LRESULT CALLBACK PreviewWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		{
			auto createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
			auto windowInstance = static_cast<PreviewWindow*>(createStruct->lpCreateParams);
			PreviewWindowStorage::AddWindow(hwnd, windowInstance);
		}
		break;
		
	case WM_SIZE:
		{
			auto window = PreviewWindowStorage::RetrieveWindow(hwnd);

			Utilities::CriticalSection::Lock lock(window->m_WindowCriticalSection);
			window->m_CrossThreadItems.m_WindowWidth = LOWORD(lParam);
			window->m_CrossThreadItems.m_WindowHeight = HIWORD(lParam);
			window->m_CrossThreadItems.m_WindowSizeDirty = true;
		}
		break;

	case WM_CLOSE:
		PreviewWindowStorage::RetrieveWindow(hwnd)->m_IsDestroyed = true;
		return 0;
	}

	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void PreviewWindow::WindowLoop()
{
	MSG msg;

	while (!m_IsDestroyed)
	{
		auto result = GetMessageW(&msg, m_Hwnd, 0, 0);
		Assert(result != -1);

		if (result != FALSE)
			DispatchMessageW(&msg);
	}
}

void PreviewWindow::ResizeSwapchain(ID3D11DeviceContext* d3d11DeviceContext)
{
	Microsoft::WRL::ComPtr<ID3D11Device> d3d11Device;
	Microsoft::WRL::ComPtr<ID3D11Resource> backBuffer;

	d3d11DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	m_BackBufferRTV = nullptr;

	auto hr = m_SwapChain->ResizeBuffers(0, m_CrossThreadItems.m_WindowWidth, m_CrossThreadItems.m_WindowHeight, DXGI_FORMAT_UNKNOWN, 0);
	Assert(SUCCEEDED(hr));

	d3d11DeviceContext->GetDevice(&d3d11Device);

	hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Resource), &backBuffer);
	Assert(SUCCEEDED(hr));

	hr = d3d11Device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_BackBufferRTV);
	Assert(SUCCEEDED(hr));
}

void PreviewWindow::UpdateScaleBuffer(ID3D11DeviceContext* d3d11DeviceContext)
{
	struct { float x, y; } scale;

	auto windowAspectRatio = static_cast<float>(m_CrossThreadItems.m_WindowWidth) / static_cast<float>(m_CrossThreadItems.m_WindowHeight);

	if (windowAspectRatio > m_CrossThreadItems.m_TextureAspectRatio)
	{
		// Vertical bars
		scale.x = m_CrossThreadItems.m_TextureAspectRatio / windowAspectRatio;
		scale.y = 1.0f;
	}
	else
	{
		// Horizontal bars
		scale.x = 1.0f;
		scale.y = windowAspectRatio / m_CrossThreadItems.m_TextureAspectRatio;
	}

	D3D11_MAPPED_SUBRESOURCE subresource;

	auto hr = d3d11DeviceContext->Map(m_ScaleBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
	Assert(SUCCEEDED(hr));

	memcpy(subresource.pData, &scale, sizeof(scale));

	d3d11DeviceContext->Unmap(m_ScaleBuffer.Get(), 0);
}

void PreviewWindow::Blit(ID3D11DeviceContext* d3d11DeviceContext, ID3D11ShaderResourceView* texture, int textureWidth, int textureHeight)
{
	uint16_t windowWidth, windowHeight;

	{
		Utilities::CriticalSection::Lock lock(m_WindowCriticalSection);

		auto newTextureAspectRatio = static_cast<float>(textureWidth) / static_cast<float>(textureHeight);

		if (m_CrossThreadItems.m_WindowSizeDirty || newTextureAspectRatio != m_CrossThreadItems.m_TextureAspectRatio)
		{
			if (m_CrossThreadItems.m_WindowSizeDirty)
			{
				m_CrossThreadItems.m_WindowSizeDirty = false;
				ResizeSwapchain(d3d11DeviceContext);
			}

			m_CrossThreadItems.m_TextureAspectRatio = newTextureAspectRatio;
			UpdateScaleBuffer(d3d11DeviceContext);
		}

		windowWidth = m_CrossThreadItems.m_WindowWidth;
		windowHeight = m_CrossThreadItems.m_WindowHeight;
	}

	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	d3d11DeviceContext->ClearRenderTargetView(m_BackBufferRTV.Get(), clearColor);

	const D3D11_VIEWPORT viewPort = { 0.0f, 0.0f, static_cast<float>(windowWidth), static_cast<float>(windowHeight), 0.0f, 1.0f };

	d3d11DeviceContext->IASetInputLayout(m_InputLayout.Get());
	d3d11DeviceContext->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &m_VertexBufferStride, &m_VertexBufferOffset);
	d3d11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	d3d11DeviceContext->VSSetConstantBuffers(0, 1, m_ScaleBuffer.GetAddressOf());
	d3d11DeviceContext->VSSetShader(m_VertexShader.Get(), nullptr, 0);
	d3d11DeviceContext->RSSetViewports(1, &viewPort);
	d3d11DeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0);
	d3d11DeviceContext->PSSetSamplers(0, 1, m_SamplerState.GetAddressOf());
	d3d11DeviceContext->PSSetShaderResources(0, 1, &texture);
	d3d11DeviceContext->OMSetRenderTargets(1, m_BackBufferRTV.GetAddressOf(), nullptr);
	d3d11DeviceContext->Draw(4, 0);

	m_SwapChain->Present(1, 0);
}