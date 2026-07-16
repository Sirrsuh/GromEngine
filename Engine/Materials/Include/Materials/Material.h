#pragma once
#include <Core/Types.h>
#include <Core/Container.h>
#include <Core/Math.h>
#include <Materials/MaterialParameter.h>
#include <Materials/ShaderPermutation.h>

namespace grom
{

class ShaderLibrary;

class Material
{
public:
	Material();
	~Material();

	void SetShaderLibrary(ShaderLibrary* library) { m_ShaderLibrary = library; }
	ShaderLibrary* GetShaderLibrary() const { return m_ShaderLibrary; }

	void SetDomain(EMaterialDomain domain) { m_Domain = domain; }
	EMaterialDomain GetDomain() const { return m_Domain; }
	void SetBlendMode(EMaterialBlendMode mode) { m_BlendMode = mode; }
	EMaterialBlendMode GetBlendMode() const { return m_BlendMode; }
	void SetShadingModel(EMaterialShadingModel model) { m_ShadingModel = model; }
	EMaterialShadingModel GetShadingModel() const { return m_ShadingModel; }
	void SetTwoSided(bool twoSided) { m_TwoSided = twoSided; }
	bool IsTwoSided() const { return m_TwoSided; }

	u32 AddParameter(const MaterialParameterDesc& desc);
	const MaterialParameterDesc& GetParameter(u32 index) const;
	u32 GetParameterCount() const { return static_cast<u32>(m_Parameters.Size()); }
	i32 FindParameter(const GString& name) const;
	u32 GetParameterBlockSize() const { return m_ParameterBlockSize; }
	void BuildParameterLayout();

	void SetFeatureFlags(u32 flags) { m_FeatureFlags = flags; }
	u32 GetFeatureFlags() const { return m_FeatureFlags; }
	void AddFeatureFlag(u32 flag) { m_FeatureFlags |= flag; }
	bool HasFeature(u32 flag) const { return (m_FeatureFlags & flag) != 0; }

	ShaderPermutation* GetPermutation(const ShaderPermutationKey& key);
	ShaderPermutation* GetOrCreatePermutation(const ShaderPermutationKey& key);

	bool CompilePermutation(ShaderPermutation* permutation);

	GString GetName() const { return m_Name; }
	void SetName(const GString& name) { m_Name = name; }

	struct ParameterBlockLayout
	{
		TArray<MaterialParameterDesc> Descs;
		u32 TotalSize = 0;
	};
	const ParameterBlockLayout& GetLayout() const { return m_Layout; }

private:
	GString m_Name;
	ShaderLibrary* m_ShaderLibrary = nullptr;
	EMaterialDomain m_Domain = EMaterialDomain::Surface;
	EMaterialBlendMode m_BlendMode = EMaterialBlendMode::Opaque;
	EMaterialShadingModel m_ShadingModel = EMaterialShadingModel::DefaultLit;
	bool m_TwoSided = false;

	TArray<MaterialParameterDesc> m_Parameters;
	u32 m_ParameterBlockSize = 0;
	u32 m_FeatureFlags = EMaterialFeatureFlags::None;

	struct PermutationEntry
	{
		ShaderPermutationKey Key;
		ShaderPermutation* Permutation = nullptr;
	};
	TArray<PermutationEntry> m_Permutations;

	ParameterBlockLayout m_Layout;
};

}
