#pragma once

#include "D3D11Context.h"
#include "NvEncode.h"
#include "Windowing\PreviewWindow.h"
#include "Utilities\Logging.h"

class SEE_Encoder
{
private:
	Utilities::Logging m_Logging;
	D3D11Context m_D3D11;
	NvEncode m_NvEncode;
	Windowing::PreviewWindow m_PreviewWindow;

public:
	inline SEE_Encoder(const wchar_t* logFileName, bool forceOverwriteLogFile) :
		m_Logging(logFileName, forceOverwriteLogFile),
		m_D3D11(m_Logging),
		m_NvEncode(m_Logging),
		m_PreviewWindow(m_Logging)
	{
		m_D3D11.PrintDeviceInfo(m_Logging);

		auto d3d11Device = m_D3D11.GetDevice();

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;

		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroStructure(&textureDesc);

		textureDesc.Width = 1;
		textureDesc.Height = 1;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

		D3D11_SUBRESOURCE_DATA textureData;

		uint32_t white = 0xFFFFFFFF;
		textureData.pSysMem = &white;
		textureData.SysMemPitch = 4;

		auto hr = d3d11Device->CreateTexture2D(&textureDesc, &textureData, &texture);
		Assert(SUCCEEDED(hr));

		m_D3D11.GetDeviceContext()->Flush();

		while (!m_PreviewWindow.IsDestroyed())
		{
			m_PreviewWindow.Blit(texture.Get());
			Sleep(2000);
		}
	}

	inline ~SEE_Encoder() { }
};

