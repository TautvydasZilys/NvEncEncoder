#pragma once

#include "Utilities\Event.h"

namespace Utilities { class Logging; }

namespace Windowing
{
	class PreviewWindowRenderer;

	class PreviewWindow
	{
		Utilities::Event m_DestroyedEvent;
		bool m_ShouldDestruct;
		PreviewWindowRenderer* m_Renderer;

	public:
		PreviewWindow(Utilities::Logging& logging);
		~PreviewWindow();

		PreviewWindow(const PreviewWindow&) = delete;
		PreviewWindow& operator=(const PreviewWindow&) = delete;

		bool IsDestroyed() const { return m_ShouldDestruct; }
		void Blit(ID3D11Texture2D* texture);
	};
}