#pragma once

#include "RHI_RefCounted.h"
#include "RHI_Enums.h"
#include <Core/Container.h>

namespace grom
{

struct TextureDesc
{
	u32 Width = 0;
	u32 Height = 0;
	u32 Depth = 1;
	u32 MipLevels = 1;
	u32 ArraySize = 1;
	EFormat Format = EFormat::Unknown;
	bool IsRenderTarget = false;
	bool IsDepthStencil = false;
	bool IsUnorderedAccess = false;
	void* InitialData = nullptr;
	GString DebugName;
};

class Texture : public RefCounted
{
public:
	virtual void* GetHandle() = 0;
	virtual TextureDesc& GetDesc() = 0;
	virtual void Upload(void* data, u32 mipLevel = 0) = 0;
	virtual ~Texture() = default;

	static Texture* Create(TextureDesc& desc, ERenderAPI api);
};

} // namespace grom
