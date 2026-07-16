#include <Materials/MaterialInstance.h>
#include <Materials/ShaderLibrary.h>
#include <cstring>
#include <cstdlib>

namespace grom
{

MaterialInstance::MaterialInstance()
{
}

MaterialInstance::~MaterialInstance()
{
	if (m_GPUBuffer)
	{
		m_GPUBuffer->Release();
		m_GPUBuffer = nullptr;
	}
}

void MaterialInstance::SetMaterial(Material* material)
{
	m_Material = material;

	u32 paramCount = m_Material ? m_Material->GetParameterCount() : 0;
	m_Overrides.Reserve(paramCount);
	m_OverrideSet.Reserve(paramCount);

	for (u32 i = 0; i < paramCount; ++i)
	{
		OverrideValue ov;
		ov.Type = EMaterialParameterType::Unknown;
		std::memset(&ov.Scalar[0], 0, sizeof(ov.Scalar));
		ov.ArrayCount = 1;
		m_Overrides.Add(ov);
		m_OverrideSet.Add(false);
	}

	m_Dirty = true;
}

void MaterialInstance::SetScalar(const GString& name, f32 value)
{
	if (!m_Material) return;

	i32 idx = m_Material->FindParameter(name);
	if (idx < 0) return;

	m_Overrides[idx].Type = EMaterialParameterType::Scalar;
	m_Overrides[idx].Scalar[0] = value;
	m_OverrideSet[idx] = true;
	m_Dirty = true;
}

void MaterialInstance::SetVector(const GString& name, const GVec4& value)
{
	if (!m_Material) return;

	i32 idx = m_Material->FindParameter(name);
	if (idx < 0) return;

	m_Overrides[idx].Type = EMaterialParameterType::Vector4;
	m_Overrides[idx].Scalar[0] = value.x;
	m_Overrides[idx].Scalar[1] = value.y;
	m_Overrides[idx].Scalar[2] = value.z;
	m_Overrides[idx].Scalar[3] = value.w;
	m_OverrideSet[idx] = true;
	m_Dirty = true;
}

void MaterialInstance::SetColor(const GString& name, const GColor& value)
{
	if (!m_Material) return;

	i32 idx = m_Material->FindParameter(name);
	if (idx < 0) return;

	m_Overrides[idx].Type = EMaterialParameterType::Color;
	m_Overrides[idx].ColorRGBA[0] = value.r;
	m_Overrides[idx].ColorRGBA[1] = value.g;
	m_Overrides[idx].ColorRGBA[2] = value.b;
	m_Overrides[idx].ColorRGBA[3] = value.a;
	m_OverrideSet[idx] = true;
	m_Dirty = true;
}

void MaterialInstance::SetTexture(const GString& name, Texture* texture)
{
	if (!m_Material) return;

	i32 idx = m_Material->FindParameter(name);
	if (idx < 0) return;

	m_Overrides[idx].Type = EMaterialParameterType::Texture2D;
	m_Overrides[idx].TexturePtr = texture;
	m_OverrideSet[idx] = true;
	m_Dirty = true;
}

void MaterialInstance::SetMatrix(const GString& name, const GMatrix4x4& value)
{
	if (!m_Material) return;

	i32 idx = m_Material->FindParameter(name);
	if (idx < 0) return;

	m_Overrides[idx].Type = EMaterialParameterType::Matrix;
	std::memcpy(m_Overrides[idx].MatrixData, &value.m[0][0], sizeof(f32) * 16);
	m_OverrideSet[idx] = true;
	m_Dirty = true;
}

f32 MaterialInstance::GetScalar(const GString& name) const
{
	if (!m_Material) return 0.0f;

	i32 idx = m_Material->FindParameter(name);
	if (idx < 0) return 0.0f;

	if (m_OverrideSet[idx])
	{
		return m_Overrides[idx].Scalar[0];
	}

	const MaterialParameterDesc& desc = m_Material->GetParameter(idx);
	GString defVal = desc.DefaultValue;
	if (!defVal.IsEmpty())
	{
		return static_cast<f32>(std::atof(defVal.c_str()));
	}

	return 0.0f;
}

GVec4 MaterialInstance::GetVector(const GString& name) const
{
	if (!m_Material) return GVec4();

	i32 idx = m_Material->FindParameter(name);
	if (idx < 0) return GVec4();

	if (m_OverrideSet[idx])
	{
		return GVec4(
			m_Overrides[idx].Scalar[0],
			m_Overrides[idx].Scalar[1],
			m_Overrides[idx].Scalar[2],
			m_Overrides[idx].Scalar[3]
		);
	}

	return GVec4();
}

Texture* MaterialInstance::GetTexture(const GString& name) const
{
	if (!m_Material) return nullptr;

	i32 idx = m_Material->FindParameter(name);
	if (idx < 0) return nullptr;

	if (m_OverrideSet[idx])
	{
		return static_cast<Texture*>(m_Overrides[idx].TexturePtr);
	}

	return nullptr;
}

bool MaterialInstance::HasOverride(const GString& name) const
{
	if (!m_Material) return false;

	i32 idx = m_Material->FindParameter(name);
	if (idx < 0) return false;

	return m_OverrideSet[idx];
}

Buffer* MaterialInstance::GetParameterBlock()
{
	return m_GPUBuffer;
}

void MaterialInstance::UpdateGPUBlock(Device* device)
{
	if (!m_Material || !m_Dirty) return;

	u32 blockSize = m_Material->GetParameterBlockSize();
	if (blockSize == 0) return;

	const Material::ParameterBlockLayout& layout = m_Material->GetLayout();

	TArray<u8> blockData;
	blockData.Reserve(blockSize);
	for (u32 i = 0; i < blockSize; ++i)
	{
		blockData.Add(0);
	}

	for (usize i = 0; i < layout.Descs.Size(); ++i)
	{
		const MaterialParameterDesc& desc = layout.Descs[i];
		u32 offset = desc.Offset;

		if (m_OverrideSet[i])
		{
			const OverrideValue& ov = m_Overrides[i];
			switch (ov.Type)
			{
				case EMaterialParameterType::Scalar:
				{
					std::memcpy(&blockData[offset], &ov.Scalar[0], sizeof(f32));
					break;
				}
				case EMaterialParameterType::Vector2:
				{
					std::memcpy(&blockData[offset], &ov.Scalar[0], sizeof(f32) * 2);
					break;
				}
				case EMaterialParameterType::Vector3:
				{
					std::memcpy(&blockData[offset], &ov.Scalar[0], sizeof(f32) * 3);
					break;
				}
				case EMaterialParameterType::Vector4:
				{
					std::memcpy(&blockData[offset], &ov.Scalar[0], sizeof(f32) * 4);
					break;
				}
				case EMaterialParameterType::Color:
				{
					f32 rgba[4] = {
						ov.ColorRGBA[0] / 255.0f,
						ov.ColorRGBA[1] / 255.0f,
						ov.ColorRGBA[2] / 255.0f,
						ov.ColorRGBA[3] / 255.0f
					};
					std::memcpy(&blockData[offset], rgba, sizeof(f32) * 4);
					break;
				}
				case EMaterialParameterType::Texture2D:
				case EMaterialParameterType::TextureCube:
				case EMaterialParameterType::Sampler:
				{
					u32 srvIndex = 0;
					std::memcpy(&blockData[offset], &srvIndex, sizeof(u32));
					break;
				}
				case EMaterialParameterType::Matrix:
				{
					std::memcpy(&blockData[offset], &ov.MatrixData[0], sizeof(f32) * 16);
					break;
				}
				default:
					break;
			}
		}
		else
		{
			GString defVal = desc.DefaultValue;
			if (!defVal.IsEmpty())
			{
				switch (desc.Type)
				{
					case EMaterialParameterType::Scalar:
					{
						f32 v = static_cast<f32>(std::atof(defVal.c_str()));
						std::memcpy(&blockData[offset], &v, sizeof(f32));
						break;
					}
					default:
						break;
				}
			}
		}
	}

	if (!m_GPUBuffer)
	{
		BufferDesc bufDesc;
		bufDesc.Size = blockSize;
		bufDesc.Stride = blockSize;
		bufDesc.Type = EBufferType::Constant;
		bufDesc.Usage = EBufferUsage::Dynamic;
		bufDesc.InitialData = blockData.Data();
		bufDesc.DebugName = m_Name + "_MaterialCB";
		m_GPUBuffer = Buffer::Create(bufDesc, device->GetAPI());
		if (m_GPUBuffer)
		{
			m_GPUBuffer->AddRef();
		}
	}
	else
	{
		m_GPUBuffer->Update(blockData.Data(), blockSize);
	}

	m_Dirty = false;
}

ShaderPermutation* MaterialInstance::GetActivePermutation()
{
	if (!m_Material) return nullptr;

	ShaderPermutationKey key;
	key.FeatureFlags = GetEffectiveFeatureFlags();
	key.QualityLevel = 0;
	key.bSkinned = (m_InstanceFlags & EMaterialFeatureFlags::Skinning) != 0;
	key.bInstanced = (m_InstanceFlags & EMaterialFeatureFlags::Instancing) != 0;
	key.bTessellated = (m_InstanceFlags & EMaterialFeatureFlags::Tessellation) != 0;

	return m_Material->GetOrCreatePermutation(key);
}

u32 MaterialInstance::GetEffectiveFeatureFlags() const
{
	u32 materialFlags = m_Material ? m_Material->GetFeatureFlags() : 0;
	return m_InstanceFlags | materialFlags;
}

}
