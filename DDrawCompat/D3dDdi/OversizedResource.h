#pragma once

#include <d3d.h>
#include <d3dumddi.h>

namespace D3dDdi
{
	class OversizedResource
	{
	public:
		OversizedResource(const D3DDDI_DEVICEFUNCS& deviceFuncs, HANDLE adapter, HANDLE device,
			D3DDDIFORMAT format, const D3DDDI_SURFACEINFO& surfaceInfo);

		HRESULT bltFrom(D3DDDIARG_BLT data);
		HRESULT bltTo(D3DDDIARG_BLT data);

		static bool isSupportedFormat(D3DDDIFORMAT format);

	private:
		HRESULT blt(D3DDDIARG_BLT& data, HANDLE& resource, RECT& rect);
		HANDLE createBltResource(RECT bltRect);

		const D3DDDI_DEVICEFUNCS& m_deviceFuncs;
		HANDLE m_adapter;
		HANDLE m_device;
		D3DDDIFORMAT m_format;
		D3DDDI_SURFACEINFO m_surfaceInfo;
	};
}
