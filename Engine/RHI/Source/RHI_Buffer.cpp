#include "RHI/RHI_Buffer.h"

#ifdef GROM_RHI_D3D11
#include "RHI/Backend/D3D11/D3D11_Buffer.h"
#include "RHI/Backend/D3D11/D3D11_Device.h"
#endif

namespace grom
{

Buffer* Buffer::Create(BufferDesc& desc, ERenderAPI api)
{
	switch (api)
	{
#ifdef GROM_RHI_D3D11
		case ERenderAPI::D3D11:
		{
			Device* dev = Device::GetActiveDevice();
			if (!dev)
				return nullptr;
			ID3D11Device* d3dDevice = static_cast<ID3D11Device*>(dev->GetNativeDevice());
			ID3D11DeviceContext* d3dContext = static_cast<ID3D11DeviceContext*>(dev->GetNativeContext());
			if (!d3dDevice || !d3dContext)
				return nullptr;
			return D3D11Buffer::Create(desc, d3dDevice, d3dContext);
		}
#endif
#ifdef GROM_RHI_D3D12
		case ERenderAPI::D3D12:
			return nullptr;
#endif
#ifdef GROM_RHI_VULKAN
		case ERenderAPI::Vulkan:
			return nullptr;
#endif
#ifdef GROM_RHI_OPENGL
		case ERenderAPI::OpenGL:
			return nullptr;
#endif
		default:
			return nullptr;
	}
}

}
