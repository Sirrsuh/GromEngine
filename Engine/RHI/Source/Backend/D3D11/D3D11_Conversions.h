#pragma once

#include "RHI/RHI_Enums.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <d3d11.h>
#include <dxgiformat.h>

namespace grom
{

inline D3D11_BIND_FLAG ToD3DBindFlags(EBufferType type)
{
	D3D11_BIND_FLAG flags = static_cast<D3D11_BIND_FLAG>(0);
	switch (type)
	{
	case EBufferType::Vertex:     flags = D3D11_BIND_VERTEX_BUFFER; break;
	case EBufferType::Index:      flags = D3D11_BIND_INDEX_BUFFER; break;
	case EBufferType::Constant:   flags = D3D11_BIND_CONSTANT_BUFFER; break;
	case EBufferType::Structured: flags = D3D11_BIND_SHADER_RESOURCE; break;
	case EBufferType::Staging:    flags = static_cast<D3D11_BIND_FLAG>(0); break;
	}
	return flags;
}

inline D3D11_USAGE ToD3DUsage(EBufferUsage usage)
{
	switch (usage)
	{
	case EBufferUsage::Default:   return D3D11_USAGE_DEFAULT;
	case EBufferUsage::Immutable: return D3D11_USAGE_IMMUTABLE;
	case EBufferUsage::Dynamic:   return D3D11_USAGE_DYNAMIC;
	case EBufferUsage::Staging:   return D3D11_USAGE_STAGING;
	default:                      return D3D11_USAGE_DEFAULT;
	}
}

inline UINT ToD3DCPUAccess(EBufferUsage usage)
{
	switch (usage)
	{
	case EBufferUsage::Dynamic: return D3D11_CPU_ACCESS_WRITE;
	case EBufferUsage::Staging: return D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	default:                    return 0;
	}
}

inline DXGI_FORMAT ToDXGIFormat(EFormat format)
{
	switch (format)
	{
	case EFormat::Unknown:              return DXGI_FORMAT_UNKNOWN;
	case EFormat::R8_UNORM:             return DXGI_FORMAT_R8_UNORM;
	case EFormat::R8G8B8A8_UNORM:       return DXGI_FORMAT_R8G8B8A8_UNORM;
	case EFormat::R16G16B16A16_FLOAT:   return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case EFormat::R32G32B32A32_FLOAT:   return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case EFormat::R32G32B32_FLOAT:      return DXGI_FORMAT_R32G32B32_FLOAT;
	case EFormat::D32_FLOAT:            return DXGI_FORMAT_D32_FLOAT;
	case EFormat::D24_UNORM_S8_UINT:    return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case EFormat::R32_FLOAT:            return DXGI_FORMAT_R32_FLOAT;
	case EFormat::R16G16_FLOAT:         return DXGI_FORMAT_R16G16_FLOAT;
	case EFormat::R11G11B10_FLOAT:      return DXGI_FORMAT_R11G11B10_FLOAT;
	case EFormat::BC1_UNORM:            return DXGI_FORMAT_BC1_UNORM;
	case EFormat::BC3_UNORM:            return DXGI_FORMAT_BC3_UNORM;
	case EFormat::BC5_UNORM:            return DXGI_FORMAT_BC5_UNORM;
	case EFormat::BC7_UNORM:            return DXGI_FORMAT_BC7_UNORM;
	default:                            return DXGI_FORMAT_UNKNOWN;
	}
}

inline D3D11_PRIMITIVE_TOPOLOGY ToD3DTopology(EPrimitiveTopology topo)
{
	switch (topo)
	{
	case EPrimitiveTopology::PointList:    return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	case EPrimitiveTopology::LineList:     return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
	case EPrimitiveTopology::TriangleList: return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	case EPrimitiveTopology::TriangleStrip:return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
	case EPrimitiveTopology::LineStrip:    return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
	default:                               return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}
}

inline D3D11_CULL_MODE ToD3DCullMode(ECullMode mode)
{
	switch (mode)
	{
	case ECullMode::None:  return D3D11_CULL_NONE;
	case ECullMode::Front: return D3D11_CULL_FRONT;
	case ECullMode::Back:  return D3D11_CULL_BACK;
	default:               return D3D11_CULL_BACK;
	}
}

inline D3D11_COMPARISON_FUNC ToD3DCompareOp(ECompareOp op)
{
	switch (op)
	{
	case ECompareOp::Never:        return D3D11_COMPARISON_NEVER;
	case ECompareOp::Less:         return D3D11_COMPARISON_LESS;
	case ECompareOp::Equal:        return D3D11_COMPARISON_EQUAL;
	case ECompareOp::LessEqual:    return D3D11_COMPARISON_LESS_EQUAL;
	case ECompareOp::Greater:      return D3D11_COMPARISON_GREATER;
	case ECompareOp::NotEqual:     return D3D11_COMPARISON_NOT_EQUAL;
	case ECompareOp::GreaterEqual: return D3D11_COMPARISON_GREATER_EQUAL;
	case ECompareOp::Always:       return D3D11_COMPARISON_ALWAYS;
	default:                       return D3D11_COMPARISON_ALWAYS;
	}
}

inline D3D11_BLEND ToD3DBlend(EBlendFactor factor)
{
	switch (factor)
	{
	case EBlendFactor::Zero:        return D3D11_BLEND_ZERO;
	case EBlendFactor::One:         return D3D11_BLEND_ONE;
	case EBlendFactor::SrcColor:    return D3D11_BLEND_SRC_COLOR;
	case EBlendFactor::InvSrcColor: return D3D11_BLEND_INV_SRC_COLOR;
	case EBlendFactor::SrcAlpha:    return D3D11_BLEND_SRC_ALPHA;
	case EBlendFactor::InvSrcAlpha: return D3D11_BLEND_INV_SRC_ALPHA;
	case EBlendFactor::DstColor:    return D3D11_BLEND_DEST_COLOR;
	case EBlendFactor::InvDstColor: return D3D11_BLEND_INV_DEST_COLOR;
	case EBlendFactor::DstAlpha:    return D3D11_BLEND_DEST_ALPHA;
	case EBlendFactor::InvDstAlpha: return D3D11_BLEND_INV_DEST_ALPHA;
	default:                        return D3D11_BLEND_ONE;
	}
}

inline D3D11_BLEND_OP ToD3DBlendOp(EBlendOp op)
{
	switch (op)
	{
	case EBlendOp::Add:         return D3D11_BLEND_OP_ADD;
	case EBlendOp::Subtract:    return D3D11_BLEND_OP_SUBTRACT;
	case EBlendOp::RevSubtract: return D3D11_BLEND_OP_REV_SUBTRACT;
	case EBlendOp::Min:         return D3D11_BLEND_OP_MIN;
	case EBlendOp::Max:         return D3D11_BLEND_OP_MAX;
	default:                    return D3D11_BLEND_OP_ADD;
	}
}

inline D3D11_FILTER ToD3DFilter(EFilter filter)
{
	switch (filter)
	{
	case EFilter::Nearest:            return D3D11_FILTER_MIN_MAG_MIP_POINT;
	case EFilter::Linear:             return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	case EFilter::Anisotropic:        return D3D11_FILTER_ANISOTROPIC;
	case EFilter::Linear_MipLinear:    return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	case EFilter::Nearest_MipNearest:  return D3D11_FILTER_MIN_MAG_MIP_POINT;
	default:                           return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	}
}

inline D3D11_TEXTURE_ADDRESS_MODE ToD3DAddressMode(EAddressMode mode)
{
	switch (mode)
	{
	case EAddressMode::Wrap:   return D3D11_TEXTURE_ADDRESS_WRAP;
	case EAddressMode::Mirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
	case EAddressMode::Clamp:  return D3D11_TEXTURE_ADDRESS_CLAMP;
	case EAddressMode::Border: return D3D11_TEXTURE_ADDRESS_BORDER;
	default:                   return D3D11_TEXTURE_ADDRESS_WRAP;
	}
}

inline bool IsDepthFormat(EFormat format)
{
	switch (format)
	{
	case EFormat::D32_FLOAT:
	case EFormat::D24_UNORM_S8_UINT:
		return true;
	default:
		return false;
	}
}

}
