#include <Materials/ShaderPermutation.h>
#include <RHI/RHI_Device.h>
#include <cstring>

namespace grom
{

ShaderPermutation::~ShaderPermutation()
{
	if (m_Pipeline)
	{
		m_Pipeline->Release();
		m_Pipeline = nullptr;
	}
	if (m_VS)
	{
		m_VS->Release();
		m_VS = nullptr;
	}
	if (m_PS)
	{
		m_PS->Release();
		m_PS = nullptr;
	}
}

bool ShaderPermutation::Key::operator==(const Key& other) const
{
	return FeatureFlags == other.FeatureFlags &&
		QualityLevel == other.QualityLevel &&
		bSkinned == other.bSkinned &&
		bInstanced == other.bInstanced &&
		bTessellated == other.bTessellated;
}

u32 ShaderPermutation::Key::Hash() const
{
	u32 hash = FeatureFlags;
	hash ^= QualityLevel + 0x9e3779b9 + (hash << 6) + (hash >> 2);
	hash ^= (static_cast<u32>(bSkinned) << 16) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
	hash ^= (static_cast<u32>(bInstanced) << 17) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
	hash ^= (static_cast<u32>(bTessellated) << 18) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
	return hash;
}

u32 ShaderPermutation::HashKey(const Key& key)
{
	return key.Hash();
}

bool ShaderPermutation::MatchesKey(const Key& a, const Key& b)
{
	return a == b;
}

bool ShaderPermutation::Compile(Shader* vs, Shader* ps)
{
	if (!vs || !ps)
	{
		return false;
	}

	m_VS = vs;
	m_PS = ps;

	m_VS->AddRef();
	m_PS->AddRef();

	PipelineDesc desc;
	desc.VS = m_VS;
	desc.PS = m_PS;
	desc.Rasterizer.CullMode = ECullMode::Back;
	desc.DepthStencil.DepthEnable = true;
	desc.DepthStencil.DepthWrite = true;
	desc.DepthStencil.DepthFunc = ECompareOp::Less;
	desc.Blend.Enable = false;
	desc.Topology = EPrimitiveTopology::TriangleList;

	m_Pipeline = Pipeline::Create(desc, ERenderAPI::D3D11);
	if (m_Pipeline)
	{
		m_Pipeline->AddRef();
	}

	return m_Pipeline != nullptr;
}

void ShaderPermutation::Bind(Device* device)
{
	if (m_Pipeline)
	{
		device->SetPipeline(m_Pipeline);
	}
}

}
