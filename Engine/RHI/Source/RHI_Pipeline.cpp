#include "RHI/RHI_Pipeline.h"

#ifdef GROM_RHI_D3D11
#include "RHI/Backend/D3D11/D3D11_Pipeline.h"
#include "RHI/Backend/D3D11/D3D11_Device.h"
#endif

#ifdef GROM_RHI_VULKAN
#include "RHI/Backend/Vulkan/Vulkan_Pipeline.h"
#include "RHI/Backend/Vulkan/Vulkan_Device.h"
#endif

namespace grom
{

Pipeline* Pipeline::Create(PipelineDesc& desc, ERenderAPI api)
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
			if (!d3dDevice)
				return nullptr;
			return D3D11Pipeline::Create(desc, d3dDevice);
		}
#endif
#ifdef GROM_RHI_D3D12
		case ERenderAPI::D3D12:
			return nullptr;
#endif
#ifdef GROM_RHI_VULKAN
		case ERenderAPI::Vulkan:
		{
			Device* dev = Device::GetActiveDevice();
			if (!dev) return nullptr;
			VkDevice vkDevice = static_cast<VkDevice>(dev->GetNativeDevice());
			VulkanDevice* vkDev = static_cast<VulkanDevice*>(dev);
			if (!vkDevice) return nullptr;
			return VulkanPipeline::Create(desc, vkDevice, vkDev->GetRenderPass());
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
