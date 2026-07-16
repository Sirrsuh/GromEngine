#pragma once

#include "RHI_RefCounted.h"
#include "RHI_Enums.h"
#include <Core/Container.h>

namespace grom
{

struct BufferDesc
{
	u32 Size = 0;
	u32 Stride = 0;
	EBufferType Type = EBufferType::Vertex;
	EBufferUsage Usage = EBufferUsage::Default;
	void* InitialData = nullptr;
	GString DebugName;
};

class Buffer : public RefCounted
{
public:
	virtual void* GetHandle() = 0;
	virtual BufferDesc& GetDesc() = 0;
	virtual void Update(void* data, u32 size, u32 offset = 0) = 0;
	virtual void* Map() = 0;
	virtual void Unmap() = 0;
	virtual ~Buffer() = default;

	static Buffer* Create(BufferDesc& desc, ERenderAPI api);
};

} // namespace grom
