#pragma once
#include <Core/Types.h>
#include <Core/Container.h>
#include <Core/Math.h>
#include <RHI/RHI_Buffer.h>
#include <RHI/RHI_Texture.h>
#include <Materials/Material.h>
#include <Materials/MaterialParameter.h>

namespace grom
{

class Device;

class MaterialInstance
{
public:
	MaterialInstance();
	~MaterialInstance();

	void SetMaterial(Material* material);
	Material* GetMaterial() const { return m_Material; }

	void SetScalar(const GString& name, f32 value);
	void SetVector(const GString& name, const GVec4& value);
	void SetColor(const GString& name, const GColor& value);
	void SetTexture(const GString& name, Texture* texture);
	void SetMatrix(const GString& name, const GMatrix4x4& value);

	f32 GetScalar(const GString& name) const;
	GVec4 GetVector(const GString& name) const;
	Texture* GetTexture(const GString& name) const;

	bool HasOverride(const GString& name) const;

	Buffer* GetParameterBlock();
	void UpdateGPUBlock(Device* device);

	ShaderPermutation* GetActivePermutation();

	void SetInstanceFeatureFlags(u32 flags) { m_InstanceFlags = flags; }
	u32 GetEffectiveFeatureFlags() const;

	GString GetName() const { return m_Name; }
	void SetName(const GString& name) { m_Name = name; }

private:
	struct OverrideValue
	{
		EMaterialParameterType Type = EMaterialParameterType::Unknown;
		union
		{
			f32 Scalar[4];
			u8 ColorRGBA[4];
			void* TexturePtr;
			f32 MatrixData[16];
		};
		u32 ArrayCount = 1;
	};

	GString m_Name;
	Material* m_Material = nullptr;
	TArray<OverrideValue> m_Overrides;
	TArray<bool> m_OverrideSet;
	Buffer* m_GPUBuffer = nullptr;
	u32 m_InstanceFlags = EMaterialFeatureFlags::None;
	bool m_Dirty = true;
};

}
