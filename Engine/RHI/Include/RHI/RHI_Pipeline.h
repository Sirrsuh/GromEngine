#pragma once

#include "RHI_RefCounted.h"
#include "RHI_Enums.h"
#include <Core/Container.h>

namespace grom
{

class Shader;
class Buffer;

struct RasterizerDesc
{
	ECullMode CullMode = ECullMode::Back;
	bool Wireframe = false;
	bool DepthClip = true;
	i32 DepthBias = 0;
	f32 DepthBiasClamp = 0.0f;
	f32 SlopeScaledDepthBias = 0.0f;
};

struct DepthStencilDesc
{
	bool DepthEnable = true;
	bool DepthWrite = true;
	ECompareOp DepthFunc = ECompareOp::Less;
	bool StencilEnable = false;
};

struct BlendDesc
{
	bool Enable = false;
	EBlendOp ColorOp = EBlendOp::Add;
	EBlendFactor SrcFactor = EBlendFactor::One;
	EBlendFactor DstFactor = EBlendFactor::Zero;
	EBlendOp AlphaOp = EBlendOp::Add;
	EBlendFactor SrcFactorAlpha = EBlendFactor::One;
	EBlendFactor DstFactorAlpha = EBlendFactor::Zero;
};

struct BufferLayout
{
	struct Element
	{
		GString Name;
		EFormat Format = EFormat::Unknown;
		u32 Offset = 0;
		u32 Slot = 0;
	};

	TArray<Element> Elements;
	u32 Stride = 0;
};

struct PipelineDesc
{
	Shader* VS = nullptr;
	Shader* PS = nullptr;
	Shader* GS = nullptr;
	Shader* HS = nullptr;
	Shader* DS = nullptr;
	Shader* CS = nullptr;
	BufferLayout InputLayout;
	RasterizerDesc Rasterizer;
	DepthStencilDesc DepthStencil;
	BlendDesc Blend;
	EPrimitiveTopology Topology = EPrimitiveTopology::TriangleList;
};

class Pipeline : public RefCounted
{
public:
	virtual void* GetHandle() = 0;
	virtual PipelineDesc& GetDesc() = 0;
	virtual ~Pipeline() = default;

	static Pipeline* Create(PipelineDesc& desc, ERenderAPI api);
};

} // namespace grom
