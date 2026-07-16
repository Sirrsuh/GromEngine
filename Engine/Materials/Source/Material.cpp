#include <Materials/Material.h>
#include <Materials/ShaderLibrary.h>

namespace grom
{

static u32 AlignUp(u32 value, u32 alignment)
{
	return (value + alignment - 1) & ~(alignment - 1);
}

static u32 GetParameterTypeSize(EMaterialParameterType type)
{
	switch (type)
	{
		case EMaterialParameterType::Scalar:    return 4;
		case EMaterialParameterType::Vector2:   return 8;
		case EMaterialParameterType::Vector3:   return 12;
		case EMaterialParameterType::Vector4:   return 16;
		case EMaterialParameterType::Color:     return 16;
		case EMaterialParameterType::Texture2D: return 4;
		case EMaterialParameterType::TextureCube: return 4;
		case EMaterialParameterType::Sampler:   return 4;
		case EMaterialParameterType::Matrix:    return 64;
		default: return 0;
	}
}

static u32 GetParameterTypeAlignment(EMaterialParameterType type)
{
	switch (type)
	{
		case EMaterialParameterType::Scalar:    return 4;
		case EMaterialParameterType::Vector2:   return 8;
		case EMaterialParameterType::Vector3:   return 16;
		case EMaterialParameterType::Vector4:   return 16;
		case EMaterialParameterType::Color:     return 16;
		case EMaterialParameterType::Texture2D: return 4;
		case EMaterialParameterType::TextureCube: return 4;
		case EMaterialParameterType::Sampler:   return 4;
		case EMaterialParameterType::Matrix:    return 16;
		default: return 1;
	}
}

Material::Material()
{
}

Material::~Material()
{
	for (usize i = 0; i < m_Permutations.Size(); ++i)
	{
		if (m_Permutations[i].Permutation)
		{
			delete m_Permutations[i].Permutation;
			m_Permutations[i].Permutation = nullptr;
		}
	}
}

u32 Material::AddParameter(const MaterialParameterDesc& desc)
{
	u32 index = static_cast<u32>(m_Parameters.Size());
	MaterialParameterDesc newDesc = desc;
	newDesc.ArrayCount = desc.ArrayCount > 0 ? desc.ArrayCount : 1;
	newDesc.Size = GetParameterTypeSize(desc.Type) * newDesc.ArrayCount;
	m_Parameters.Add(newDesc);
	BuildParameterLayout();
	return index;
}

const MaterialParameterDesc& Material::GetParameter(u32 index) const
{
	return m_Parameters[index];
}

i32 Material::FindParameter(const GString& name) const
{
	for (usize i = 0; i < m_Parameters.Size(); ++i)
	{
		if (m_Parameters[i].Name == name)
		{
			return static_cast<i32>(i);
		}
	}
	return -1;
}

void Material::BuildParameterLayout()
{
	u32 currentOffset = 0;

	for (usize i = 0; i < m_Parameters.Size(); ++i)
	{
		u32 alignment = GetParameterTypeAlignment(m_Parameters[i].Type);
		currentOffset = AlignUp(currentOffset, alignment);
		m_Parameters[i].Offset = currentOffset;
		currentOffset += m_Parameters[i].Size;
	}

	m_ParameterBlockSize = AlignUp(currentOffset, 16);

	m_Layout.Descs.Clear();
	for (usize i = 0; i < m_Parameters.Size(); ++i)
	{
		m_Layout.Descs.Add(m_Parameters[i]);
	}
	m_Layout.TotalSize = m_ParameterBlockSize;
}

ShaderPermutation* Material::GetPermutation(const ShaderPermutationKey& key)
{
	for (usize i = 0; i < m_Permutations.Size(); ++i)
	{
		if (ShaderPermutation::MatchesKey(m_Permutations[i].Key, key))
		{
			return m_Permutations[i].Permutation;
		}
	}
	return nullptr;
}

ShaderPermutation* Material::GetOrCreatePermutation(const ShaderPermutationKey& key)
{
	ShaderPermutation* existing = GetPermutation(key);
	if (existing)
	{
		return existing;
	}

	ShaderPermutation* permutation = new ShaderPermutation();
	permutation->m_Key = key;

	CompilePermutation(permutation);

	PermutationEntry entry;
	entry.Key = key;
	entry.Permutation = permutation;
	m_Permutations.Add(entry);

	return permutation;
}

bool Material::CompilePermutation(ShaderPermutation* permutation)
{
	if (!m_ShaderLibrary)
	{
		return false;
	}

	ShaderPermutationKey key = permutation->GetKey();

	Shader* vs = nullptr;
	Shader* ps = nullptr;

	for (u32 i = 0; i < m_ShaderLibrary->GetShaderCount(); ++i)
	{
		if (m_ShaderLibrary->GetShaders(i, key, vs, ps) && vs && ps)
		{
			return permutation->Compile(vs, ps);
		}
	}

	return false;
}

}
