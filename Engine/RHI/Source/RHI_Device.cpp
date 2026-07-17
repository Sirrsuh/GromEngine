#include "RHI/RHI_Device.h"

#ifdef GROM_RHI_D3D11
#include "RHI/Backend/D3D11/D3D11_Device.h"
#endif

#ifdef GROM_RHI_D3D12
#include "RHI/Backend/D3D12/D3D12_Device.h"
#endif

#ifdef GROM_RHI_VULKAN
#include "RHI/Backend/Vulkan/Vulkan_Device.h"
#endif

namespace grom
{

static Device* g_ActiveDevice = nullptr;

Device* Device::GetActiveDevice()
{
	return g_ActiveDevice;
}

Device* Device::Create(DeviceDesc& desc)
{
	switch (desc.API)
	{
#ifdef GROM_RHI_D3D11
		case ERenderAPI::D3D11:
		{
			g_ActiveDevice = D3D11Device::Create(desc);
			return g_ActiveDevice;
		}
#endif
#ifdef GROM_RHI_D3D12
		case ERenderAPI::D3D12:
		{
			g_ActiveDevice = D3D12Device::Create(desc);
			return g_ActiveDevice;
		}
#endif
#ifdef GROM_RHI_VULKAN
		case ERenderAPI::Vulkan:
		{
			g_ActiveDevice = VulkanDevice::Create(desc);
			return g_ActiveDevice;
		}
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
