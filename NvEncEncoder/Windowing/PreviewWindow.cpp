#include "PrecompiledHeader.h"
#include "Core\D3D11Context.h"
#include "PreviewWindow.h"
#include "PreviewWindowRenderer.h"

using namespace Windowing;

struct WindowThreadContext
{
	PreviewWindow& previewWindow;
	Utilities::Event& windowCreationEvent;
	Utilities::Logging& logging;

	inline WindowThreadContext(PreviewWindow& previewWindow, Utilities::Event& windowCreationEvent, Utilities::Logging& logging) :
		previewWindow(previewWindow), windowCreationEvent(windowCreationEvent), logging(logging)
	{
	}

	WindowThreadContext(const WindowThreadContext&) = delete;
	WindowThreadContext operator=(const WindowThreadContext&) = delete;
};

PreviewWindow::PreviewWindow(Utilities::Logging& logging)
{
	Utilities::Event windowCreationEvent;
	WindowThreadContext windowThreadContext(*this, windowCreationEvent, logging);

	auto windowThreadEntry = [](LPVOID lpThreadParameter) -> DWORD
	{
		WindowThreadContext* context = static_cast<WindowThreadContext*>(lpThreadParameter);
		auto& previewWindow = context->previewWindow;

		try
		{
			PreviewWindowRenderer windowRenderer(previewWindow.m_DestroyedEvent, previewWindow.m_ShouldDestruct, context->logging);
			previewWindow.m_Renderer = &windowRenderer;

			// NOTE: after this event is set, can't reference context anymore!
			context->windowCreationEvent.Set();
			context = nullptr;

			windowRenderer.WindowLoop();
		}
		catch (std::runtime_error&)
		{
			if (context != nullptr)
			{
				context->windowCreationEvent.Set();	
				context = nullptr;
			}

			previewWindow.m_ShouldDestruct = true;
			previewWindow.m_DestroyedEvent.Set();
		}

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
	if (!m_ShouldDestruct)
		m_Renderer->Destruct();

	m_DestroyedEvent.Wait();
}

void PreviewWindow::Blit(ID3D11Texture2D* texture)
{
	m_Renderer->Blit(texture);
}