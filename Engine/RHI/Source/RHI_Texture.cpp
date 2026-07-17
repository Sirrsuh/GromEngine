#include "RHI/RHI_Texture.h"

#ifdef GROM_RHI_D3D11
#include "RHI/Backend/D3D11/D3D11_Texture.h"
#include "RHI/Backend/D3D11/D3D11_Device.h"
#endif

#ifdef GROM_RHI_D3D12
#include "RHI/Backend/D3D12/D3D12_Texture.h"
#include "RHI/Backend/D3D12/D3D12_Device.h"
#endif

#ifdef GROM_RHI_VULKAN
#include "RHI/Backend/Vulkan/Vulkan_Texture.h"
#include "RHI/Backend/Vulkan/Vulkan_Device.h"
#endif

namespace grom
{

Texture* Texture::Create(TextureDesc& desc, ERenderAPI api)
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
			return D3D11Texture::Create(desc, d3dDevice);
		}
#endif
#ifdef GROM_RHI_D3D12
		case ERenderAPI::D3D12:
		{
			Device* dev = Device::GetActiveDevice();
			if (!dev) return nullptr;
			D3D12Device* d3d12Dev = static_cast<D3D12Device*>(dev);
			if (!d3d12Dev) return nullptr;
			ID3D12Device* d3d12Device = d3d12Dev->GetD3D12Device().Get();
			if (!d3d12Device) return nullptr;
			return D3D12Texture::Create(desc, d3d12Device, d3d12Dev->GetCommandList().Get());
		}
#endif
#ifdef GROM_RHI_VULKAN
		case ERenderAPI::Vulkan:
		{
			Device* dev = Device::GetActiveDevice();
			if (!dev) return nullptr;
			VkDevice vkDevice = static_cast<VkDevice>(dev->GetNativeDevice());
			VulkanDevice* vkDev = static_cast<VulkanDevice*>(dev);
			VkPhysicalDevice physDevice = vkDev->GetPhysicalDevice();
			if (!vkDevice) return nullptr;
			return VulkanTexture::Create(desc, vkDevice, physDevice);
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
