#include "PrecompiledHeader.h"
#include "D3D11Context.h"
#include "PreviewWindow.h"
#include "Shaders\PreviewWindowPixelShader.h"
#include "Shaders\PreviewWindowVertexShader.h"
#include "Utilities\CriticalSection.h"

Utilities::CriticalSection g_HwndMapCriticalSection;
std::unordered_map<HWND, PreviewWindow*> g_HwndMap;

const int kInitialWidth = 800;
const int kInitialHeight = 450;

PreviewWindow::PreviewWindow(D3D11Context& d3d11Context) :
	m_IsDestroyed(false)
{
	auto hwnd = CreateOSWindow();
	CreateD3D11Resources(hwnd, d3d11Context);
}

PreviewWindow::~PreviewWindow()
{
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

HWND PreviewWindow::CreateOSWindow()
{
	static ATOM s_WindowClassAtom = CreateWindowClass();
	
	auto hwnd = CreateWindowExW(WS_EX_APPWINDOW, reinterpret_cast<LPWSTR>(s_WindowClassAtom), L"NEE Preview", WS_CAPTION | WS_SIZEBOX | WS_VISIBLE,
		200, 200, kInitialWidth, kInitialHeight, nullptr, nullptr, GetModuleHandleW(nullptr), this);
	Assert(hwnd != nullptr);

	return hwnd;
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

void PreviewWindow::CreateD3D11Resources(HWND hwnd, D3D11Context& d3d11Context)
{	
	auto d3d11Device = d3d11Context.GetDevice();

	CreateSwapChain(hwnd, d3d11Device, m_SwapChain.ReleaseAndGetAddressOf());
	CreateShaders(d3d11Device, m_VertexShader.ReleaseAndGetAddressOf(), m_PixelShader.ReleaseAndGetAddressOf());
}

LRESULT CALLBACK PreviewWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		{
			Utilities::CriticalSection::Lock lock(g_HwndMapCriticalSection);
			auto createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
			auto windowInstance = static_cast<PreviewWindow*>(createStruct->lpCreateParams);
			g_HwndMap.insert(std::make_pair(hwnd, windowInstance));
		}
	}

	return DefWindowProcW(hwnd, msg, wParam, lParam);
}