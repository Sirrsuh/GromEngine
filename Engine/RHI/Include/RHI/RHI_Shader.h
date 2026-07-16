#pragma once

#include "RHI_RefCounted.h"
#include "RHI_Enums.h"
#include <Core/Container.h>

namespace grom
{

class ShaderDesc
{
public:
	GString EntryPoint;
	GString Source;
	EShaderType Type = EShaderType::Vertex;
	EFormat Format = EFormat::Unknown;
};

class Shader : public RefCounted
{
public:
	virtual void* GetHandle() = 0;
	virtual ShaderDesc& GetDesc() = 0;
	virtual ~Shader() = default;

	GROM_RHI_API static Shader* Create(ShaderDesc& desc, ERenderAPI api);
};

} // namespace grom
