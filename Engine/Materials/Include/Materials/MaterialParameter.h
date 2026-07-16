#pragma once
#include <Core/Types.h>
#include <Core/Container.h>
#include <Core/Math.h>
#include <RHI/RHI_Enums.h>

namespace grom
{

enum class EMaterialParameterType
{
	Scalar,
	Vector2,
	Vector3,
	Vector4,
	Color,
	Texture2D,
	TextureCube,
	Sampler,
	Matrix,
	Unknown
};

struct MaterialParameterDesc
{
	GString Name;
	EMaterialParameterType Type;
	u32 Offset;
	u32 Size;
	u32 ArrayCount;
	GString DefaultValue;
	GString Description;
};

enum class EMaterialDomain
{
	Surface,
	PostProcess,
	UserInterface,
	DeferredDecal,
	LightFunction
};

enum class EMaterialBlendMode
{
	Opaque,
	Masked,
	Translucent,
	Additive,
	Modulate,
	AlphaComposite
};

enum class EMaterialShadingModel
{
	DefaultLit,
	Subsurface,
	ClearCoat,
	Hair,
	Cloth,
	Eye,
	Unlit
};

struct EMaterialFeatureFlags
{
	static constexpr u32 None           = 0;
	static constexpr u32 NormalMap      = 1 << 0;
	static constexpr u32 RoughnessMap   = 1 << 1;
	static constexpr u32 MetallicMap    = 1 << 2;
	static constexpr u32 AmbientOcclusionMap = 1 << 3;
	static constexpr u32 EmissiveMap    = 1 << 4;
	static constexpr u32 OpacityMap     = 1 << 5;
	static constexpr u32 DisplacementMap = 1 << 6;
	static constexpr u32 VertexColors   = 1 << 7;
	static constexpr u32 Anisotropy     = 1 << 8;
	static constexpr u32 ClearCoat      = 1 << 9;
	static constexpr u32 Subsurface     = 1 << 10;
	static constexpr u32 DualSpecular   = 1 << 11;
	static constexpr u32 Tessellation   = 1 << 12;
	static constexpr u32 Instancing     = 1 << 13;
	static constexpr u32 Skinning       = 1 << 14;
	static constexpr u32 MorphTargets   = 1 << 15;
};

}
