#pragma once
#include <Core/Types.h>
#include <Core/Container.h>
#include <RHI/RHI_Shader.h>
#include <RHI/RHI_Pipeline.h>

namespace grom
{

class ShaderPermutation
{
public:
	ShaderPermutation() = default;
	~ShaderPermutation();

	struct Key
	{
		u32 FeatureFlags = 0;
		u32 QualityLevel = 0;
		bool bSkinned = false;
		bool bInstanced = false;
		bool bTessellated = false;

		bool operator==(const Key& other) const;
		bool operator!=(const Key& other) const { return !(*this == other); }
		u32 Hash() const;
	};

	bool Compile(Shader* vs, Shader* ps);
	void Bind(class Device* device);

	Shader* GetVS() const { return m_VS; }
	Shader* GetPS() const { return m_PS; }
	Pipeline* GetPipeline() const { return m_Pipeline; }
	Key GetKey() const { return m_Key; }

	static u32 HashKey(const Key& key);
	static bool MatchesKey(const Key& a, const Key& b);

private:
	Shader* m_VS = nullptr;
	Shader* m_PS = nullptr;
	Pipeline* m_Pipeline = nullptr;
	Key m_Key;

	friend class ShaderLibrary;
	friend class Material;
};

using ShaderPermutationKey = ShaderPermutation::Key;

}
