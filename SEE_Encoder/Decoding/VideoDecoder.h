#pragma once

#include "Utilities\Com\ComPtr.h"

namespace Decoding
{
	class MediaFoundationContext;

	class VideoDecoder
	{
	private:
		Utilities::Com::ComPtr<IMFMediaSession> m_Session;
		Utilities::Com::ComPtr<IMFMediaSource> m_MediaSource;

	public:
		VideoDecoder(const MediaFoundationContext& mfContext, const wchar_t* path);
		~VideoDecoder();

		VideoDecoder(const VideoDecoder&) = delete;
		VideoDecoder& operator=(const VideoDecoder&) = delete;
	};
}