#pragma once

#include <Core/Types.h>

namespace grom
{

enum class ERenderAPI
{
	D3D11,
	D3D12,
	Vulkan,
	OpenGL,
	None
};

enum class EFormat
{
	Unknown,
	R8_UNORM,
	R8G8B8A8_UNORM,
	R16G16B16A16_FLOAT,
	R32G32B32A32_FLOAT,
	D32_FLOAT,
	D24_UNORM_S8_UINT,
	R32_FLOAT,
	R16G16_FLOAT,
	R32G32_FLOAT,
	R11G11B10_FLOAT,
	BC1_UNORM,
	BC3_UNORM,
	BC5_UNORM,
	BC7_UNORM
};

enum class EBufferType
{
	Vertex,
	Index,
	Constant,
	Structured,
	Staging
};

enum class EBufferUsage
{
	Default,
	Immutable,
	Dynamic,
	Staging
};

enum class EShaderType
{
	Vertex,
	Pixel,
	Geometry,
	Compute,
	Hull,
	Domain
};

enum class ETopologyType
{
	PointList,
	LineList,
	TriangleList,
	TriangleStrip
};

enum class EFilter
{
	Nearest,
	Linear,
	Anisotropic,
	Linear_MipLinear,
	Nearest_MipNearest
};

enum class EAddressMode
{
	Wrap,
	Mirror,
	Clamp,
	Border
};

enum class ECompareOp
{
	Never,
	Less,
	Equal,
	LessEqual,
	Greater,
	NotEqual,
	GreaterEqual,
	Always
};

enum class EBlendOp
{
	Add,
	Subtract,
	RevSubtract,
	Min,
	Max
};

enum class EBlendFactor
{
	Zero,
	One,
	SrcColor,
	InvSrcColor,
	SrcAlpha,
	InvSrcAlpha,
	DstColor,
	InvDstColor,
	DstAlpha,
	InvDstAlpha
};

enum class ECullMode
{
	None,
	Front,
	Back
};

enum class EPrimitiveTopology
{
	PointList,
	LineList,
	TriangleList,
	TriangleStrip,
	LineStrip
};

enum class EResourceState
{
	Undefined,
	VertexBuffer,
	IndexBuffer,
	ConstantBuffer,
	ShaderResource,
	UnorderedAccess,
	RenderTarget,
	DepthStencil,
	Present,
	CopySrc,
	CopyDst
};

struct ViewportDesc
{
	f32 TopLeftX = 0.0f;
	f32 TopLeftY = 0.0f;
	f32 Width = 0.0f;
	f32 Height = 0.0f;
	f32 MinDepth = 0.0f;
	f32 MaxDepth = 1.0f;
};

struct Rect2D
{
	i32 Left = 0;
	i32 Top = 0;
	i32 Right = 0;
	i32 Bottom = 0;
};

} // namespace grom
