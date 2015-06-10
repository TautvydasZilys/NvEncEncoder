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
	Utilities::Event& windowCreationEvent;
	D3D11Context& d3d11Context;
	PreviewWindow& previewWindow;

	inline WindowThreadContext(Utilities::Event& windowCreationEvent, D3D11Context& d3d11Context, PreviewWindow& previewWindow) :
		windowCreationEvent(windowCreationEvent), d3d11Context(d3d11Context), previewWindow(previewWindow)
	{
	}
};

static const int kInitialWidth = 800;
static const int kInitialHeight = 450;

PreviewWindow::PreviewWindow(D3D11Context& d3d11Context) :
	m_IsDestroyed(false), m_DestroyedEvent(true)
{
	Utilities::Event windowCreationEvent;
	WindowThreadContext windowThreadContext(windowCreationEvent, d3d11Context, *this);

	auto windowThreadEntry = [](LPVOID lpThreadParameter) -> DWORD
	{
		WindowThreadContext& context = *static_cast<WindowThreadContext*>(lpThreadParameter);
		auto& previewWindow = context.previewWindow;

		previewWindow.CreateOSWindow();
		previewWindow.CreateD3D11Resources(context.d3d11Context);

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
}

PreviewWindow::~PreviewWindow()
{
	if (!m_IsDestroyed)
		m_IsDestroyed = true;

	m_DestroyedEvent.Wait();
	PreviewWindowStorage::RemoveWindow(m_Hwnd);
}

void PreviewWindow::Cleanup()
{
	Utilities::CriticalSection::Lock lock(m_RenderCriticalSection);
	Assert(m_IsDestroyed);

	m_SwapChain = nullptr;
	m_VertexShader = nullptr;
	m_PixelShader = nullptr;

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

	m_Hwnd = CreateWindowExW(WS_EX_APPWINDOW, reinterpret_cast<LPWSTR>(s_WindowClassAtom), L"NEE Preview", kWindowStyle,
		200, 200, kInitialWidth, kInitialHeight, nullptr, nullptr, GetModuleHandleW(nullptr), this);
	Assert(m_Hwnd != nullptr);
}

static inline void CreateSwapChain(HWND hwnd, ID3D11Device* d3d11Device, IDXGISwapChain** swapChain)
{
	Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
	Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
	Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;

	auto hr = d3d11Device->QueryInterface(dxgiDevice.ReleaseAndGetAddressOf());
	Assert(SUCCEEDED(hr));

	hr = dxgiDevice->GetAdapter(dxgiAdapter.ReleaseAndGetAddressOf());
	Assert(SUCCEEDED(hr));

	hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), &dxgiFactory);
	Assert(SUCCEEDED(hr));

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroStructure(&swapChainDesc);

	swapChainDesc.BufferDesc.Width = kInitialWidth;
	swapChainDesc.BufferDesc.Height = kInitialHeight;
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
}

static inline void CreateShaders(ID3D11Device* d3d11Device, ID3D11VertexShader** vertexShader, ID3D11PixelShader** pixelShader)
{
	auto hr = d3d11Device->CreateVertexShader(g_PreviewWindowVertexShader, sizeof(g_PreviewWindowVertexShader), nullptr, vertexShader);
	Assert(SUCCEEDED(hr));

	hr = d3d11Device->CreatePixelShader(g_PreviewWindowPixelShader, sizeof(g_PreviewWindowPixelShader), nullptr, pixelShader);
	Assert(SUCCEEDED(hr));
}

void PreviewWindow::CreateD3D11Resources(D3D11Context& d3d11Context)
{	
	auto d3d11Device = d3d11Context.GetDevice();

	CreateSwapChain(m_Hwnd, d3d11Device, m_SwapChain.ReleaseAndGetAddressOf());
	CreateShaders(d3d11Device, m_VertexShader.ReleaseAndGetAddressOf(), m_PixelShader.ReleaseAndGetAddressOf());
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

	case WM_DESTROY:
		PreviewWindowStorage::RetrieveWindow(hwnd)->m_IsDestroyed = true;
		break;
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