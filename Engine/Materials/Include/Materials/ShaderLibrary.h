#pragma once
#include <Core/Types.h>
#include <Core/Container.h>
#include <Core/Math.h>
#include <RHI/RHI_Shader.h>
#include <RHI/RHI_Device.h>
#include <Materials/ShaderPermutation.h>

namespace grom {

struct ShaderSourceEntry {
    GString Name;
    GString FilePath;
    GString EntryPointVS;
    GString EntryPointPS;
    GString AdditionalDefines;
    u32 FeatureFlags;
    bool bCompiled = false;
    bool bHasErrors = false;
    GString LastError;
    u64 LastModifiedTimestamp = 0;
    u64 LastCheckedTimestamp = 0;

    struct CompiledEntry {
        ShaderPermutationKey Key;
        Shader* VS = nullptr;
        Shader* PS = nullptr;
    };
    TArray<CompiledEntry> CompiledPermutations;
};

class ShaderLibrary {
public:
    ShaderLibrary();
    ~ShaderLibrary();

    u32 RegisterShader(const ShaderSourceEntry& entry);
    void RemoveShader(u32 index);
    ShaderSourceEntry* GetShaderEntry(u32 index);
    u32 GetShaderCount() const { return static_cast<u32>(m_Shaders.Size()); }

    bool CompileShader(u32 index, const ShaderPermutationKey& key);
    bool CompileAllPermutations(u32 index);
    bool CompileAllShaders();

    Shader* GetVS(u32 index, const ShaderPermutationKey& key);
    Shader* GetPS(u32 index, const ShaderPermutationKey& key);
    bool GetShaders(u32 index, const ShaderPermutationKey& key, Shader*& outVS, Shader*& outPS);

    void CheckForChanges();
    void SetHotReloadEnabled(bool enabled) { m_HotReloadEnabled = enabled; }
    bool IsHotReloadEnabled() const { return m_HotReloadEnabled; }
    void SetAutoRecompileOnError(bool enabled) { m_AutoRecompileOnError = enabled; }

    void SetDevice(class Device* device) { m_Device = device; }
    class Device* GetDevice() const { return m_Device; }

    const GString& GetLastCompileError() const { return m_LastError; }
    bool HasCompileErrors() const { return m_bHasCompileErrors; }
    u32 GetErrorCount() const { return m_ErrorCount; }

    void AddGlobalDefine(const GString& name, const GString& value);
    void RemoveGlobalDefine(const GString& name);
    void ClearGlobalDefines();
    GString GetGlobalDefinesString() const;

    void SetShaderRootPath(const GString& path) { m_ShaderRootPath = path; }
    GString GetShaderRootPath() const { return m_ShaderRootPath; }

    static ShaderLibrary& Get();

private:
    GString BuildCompileArgs(u32 index, const ShaderPermutationKey& key);
    GString ReadFile(const GString& path);
    u64 GetFileTimestamp(const GString& path);
    bool CompileShaderFromFile(u32 index, const ShaderPermutationKey& key, Shader*& outVS, Shader*& outPS);
    bool CompileFromSource(const GString& name, const GString& source, const GString& entryVS, const GString& entryPS,
                          const GString& defines, Shader*& outVS, Shader*& outPS);

    TArray<ShaderSourceEntry> m_Shaders;
    TArray<GString> m_ShaderPaths;
    TArray<GString> m_GlobalDefines;
    class Device* m_Device = nullptr;
    GString m_ShaderRootPath;
    GString m_LastError;
    u32 m_ErrorCount = 0;
    bool m_HotReloadEnabled = true;
    bool m_AutoRecompileOnError = true;
    bool m_bHasCompileErrors = false;
    f64 m_LastHotReloadCheck = 0.0;
    f64 m_HotReloadInterval = 1.0;

    static ShaderLibrary* s_Instance;
};

}
