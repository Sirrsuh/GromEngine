#include "Materials/ShaderLibrary.h"

#include <Windows.h>
#include <cstdio>

namespace grom {

ShaderLibrary* ShaderLibrary::s_Instance = nullptr;

static constexpr u32 FEATURE_BIT_COUNT = 16;

static const char* FeatureFlagNames[FEATURE_BIT_COUNT] = {
    "FEATURE_NORMAL_MAP",
    "FEATURE_ROUGHNESS_MAP",
    "FEATURE_METALLIC_MAP",
    "FEATURE_AO_MAP",
    "FEATURE_EMISSIVE_MAP",
    "FEATURE_OPACITY_MAP",
    "FEATURE_DISPLACEMENT_MAP",
    "FEATURE_VERTEX_COLORS",
    "FEATURE_ANISOTROPY",
    "FEATURE_CLEAR_COAT",
    "FEATURE_SUBSURFACE",
    "FEATURE_DUAL_SPECULAR",
    "FEATURE_TESSELLATION",
    "FEATURE_INSTANCING",
    "FEATURE_SKINNING",
    "FEATURE_MORPH_TARGETS"
};

ShaderLibrary::ShaderLibrary()
{
    s_Instance = this;
}

ShaderLibrary::~ShaderLibrary()
{
    for (u32 i = 0; i < m_Shaders.Size(); ++i)
    {
        for (u32 j = 0; j < m_Shaders[i].CompiledPermutations.Size(); ++j)
        {
            ShaderSourceEntry::CompiledEntry& entry = m_Shaders[i].CompiledPermutations[j];
            if (entry.VS) entry.VS->Release();
            if (entry.PS) entry.PS->Release();
        }
    }

    if (s_Instance == this)
        s_Instance = nullptr;
}

ShaderLibrary& ShaderLibrary::Get()
{
    if (!s_Instance)
        s_Instance = new ShaderLibrary();
    return *s_Instance;
}

u32 ShaderLibrary::RegisterShader(const ShaderSourceEntry& entry)
{
    u32 index = static_cast<u32>(m_Shaders.Size());
    m_Shaders.Add(entry);
    return index;
}

void ShaderLibrary::RemoveShader(u32 index)
{
    if (index >= m_Shaders.Size())
        return;

    for (u32 j = 0; j < m_Shaders[index].CompiledPermutations.Size(); ++j)
    {
        ShaderSourceEntry::CompiledEntry& entry = m_Shaders[index].CompiledPermutations[j];
        if (entry.VS) entry.VS->Release();
        if (entry.PS) entry.PS->Release();
    }

    m_Shaders.RemoveAt(index);
}

ShaderSourceEntry* ShaderLibrary::GetShaderEntry(u32 index)
{
    if (index >= m_Shaders.Size())
        return nullptr;
    return &m_Shaders[index];
}

GString ShaderLibrary::BuildCompileArgs(u32 index, const ShaderPermutationKey& key)
{
    GString defines;

    for (u32 i = 0; i < m_GlobalDefines.Size(); ++i)
    {
        defines += "#define ";
        defines += m_GlobalDefines[i];
        defines += "\n";
    }

    u32 combinedFlags = m_Shaders[index].FeatureFlags | key.FeatureFlags;
    for (u32 i = 0; i < FEATURE_BIT_COUNT; ++i)
    {
        bool enabled = (combinedFlags & (1u << i)) != 0;
        defines += "#define ";
        defines += FeatureFlagNames[i];
        defines += enabled ? " 1\n" : " 0\n";
    }

    defines += "#define QUALITY_LEVEL ";
    defines += GString::Format("%u", key.QualityLevel);
    defines += "\n";

    if (key.bSkinned)
        defines += "#define SKINNED 1\n";
    else
        defines += "#define SKINNED 0\n";

    if (key.bInstanced)
        defines += "#define INSTANCED 1\n";
    else
        defines += "#define INSTANCED 0\n";

    if (key.bTessellated)
        defines += "#define TESSELLATED 1\n";
    else
        defines += "#define TESSELLATED 0\n";

    if (!m_Shaders[index].AdditionalDefines.IsEmpty())
    {
        defines += m_Shaders[index].AdditionalDefines;
        defines += "\n";
    }

    return defines;
}

GString ShaderLibrary::ReadFile(const GString& path)
{
    FILE* f = nullptr;
    fopen_s(&f, path.c_str(), "rb");
    if (!f)
        return {};

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0)
    {
        fclose(f);
        return {};
    }

    TArray<char> buffer(static_cast<usize>(size + 1));
    fread(buffer.Data(), 1, size, f);
    buffer[size] = '\0';
    fclose(f);

    return GString(buffer.Data(), static_cast<usize>(size));
}

u64 ShaderLibrary::GetFileTimestamp(const GString& path)
{
    WIN32_FILE_ATTRIBUTE_DATA data{};
    if (!GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &data))
        return 0;

    ULARGE_INTEGER time;
    time.HighPart = data.ftLastWriteTime.dwHighDateTime;
    time.LowPart = data.ftLastWriteTime.dwLowDateTime;
    return time.QuadPart;
}

bool ShaderLibrary::CompileFromSource(const GString& name, const GString& source, const GString& entryVS, const GString& entryPS,
                                     const GString& defines, Shader*& outVS, Shader*& outPS)
{
    outVS = nullptr;
    outPS = nullptr;

    if (!m_Device)
    {
        m_LastError = "No device set on ShaderLibrary";
        return false;
    }

    GString fullSource = defines + source;

    ShaderDesc vsDesc;
    vsDesc.EntryPoint = entryVS.IsEmpty() ? "main" : entryVS;
    vsDesc.Source = fullSource;
    vsDesc.Type = EShaderType::Vertex;

    outVS = Shader::Create(vsDesc, m_Device->GetAPI());
    if (!outVS)
    {
        m_LastError = GString::Format("VS compile failed for '%s'", name.c_str());
        return false;
    }

    ShaderDesc psDesc;
    psDesc.EntryPoint = entryPS.IsEmpty() ? "main" : entryPS;
    psDesc.Source = fullSource;
    psDesc.Type = EShaderType::Pixel;

    outPS = Shader::Create(psDesc, m_Device->GetAPI());
    if (!outPS)
    {
        outVS->Release();
        outVS = nullptr;
        m_LastError = GString::Format("PS compile failed for '%s'", name.c_str());
        return false;
    }

    return true;
}

bool ShaderLibrary::CompileShaderFromFile(u32 index, const ShaderPermutationKey& key, Shader*& outVS, Shader*& outPS)
{
    ShaderSourceEntry& entry = m_Shaders[index];

    GString fullPath = m_ShaderRootPath + "/" + entry.FilePath;
    GString source = ReadFile(fullPath);
    if (source.IsEmpty())
    {
        m_LastError = GString::Format("Failed to read shader file: %s", fullPath.c_str());
        return false;
    }

    GString defines = BuildCompileArgs(index, key);
    return CompileFromSource(entry.Name, source, entry.EntryPointVS, entry.EntryPointPS, defines, outVS, outPS);
}

bool ShaderLibrary::CompileShader(u32 index, const ShaderPermutationKey& key)
{
    if (index >= m_Shaders.Size())
    {
        m_LastError = "Invalid shader index";
        return false;
    }

    ShaderSourceEntry& entry = m_Shaders[index];

    for (u32 i = 0; i < entry.CompiledPermutations.Size(); ++i)
    {
        if (ShaderPermutation::MatchesKey(entry.CompiledPermutations[i].Key, key))
        {
            if (entry.CompiledPermutations[i].VS) entry.CompiledPermutations[i].VS->Release();
            if (entry.CompiledPermutations[i].PS) entry.CompiledPermutations[i].PS->Release();
            entry.CompiledPermutations.RemoveAt(i);
            break;
        }
    }

    Shader* vs = nullptr;
    Shader* ps = nullptr;

    bool success = CompileShaderFromFile(index, key, vs, ps);

    if (success && vs && ps)
    {
        ShaderSourceEntry::CompiledEntry compiled;
        compiled.Key = key;
        compiled.VS = vs;
        compiled.PS = ps;
        entry.CompiledPermutations.Add(compiled);
        entry.bCompiled = true;
        entry.bHasErrors = false;
        entry.LastError.Clear();
        entry.LastModifiedTimestamp = GetFileTimestamp(m_ShaderRootPath + "/" + entry.FilePath);
        m_bHasCompileErrors = false;
        return true;
    }
    else
    {
        entry.bHasErrors = true;
        entry.LastError = m_LastError;
        m_ErrorCount++;
        m_bHasCompileErrors = true;
        return false;
    }
}

bool ShaderLibrary::CompileAllPermutations(u32 index)
{
    if (index >= m_Shaders.Size())
        return false;

    bool allSuccess = true;
    ShaderPermutationKey defaultKey{};
    if (!CompileShader(index, defaultKey))
        allSuccess = false;

    return allSuccess;
}

bool ShaderLibrary::CompileAllShaders()
{
    bool allSuccess = true;
    for (u32 i = 0; i < m_Shaders.Size(); ++i)
    {
        if (!CompileAllPermutations(i))
            allSuccess = false;
    }
    return allSuccess;
}

Shader* ShaderLibrary::GetVS(u32 index, const ShaderPermutationKey& key)
{
    if (index >= m_Shaders.Size())
        return nullptr;

    ShaderSourceEntry& entry = m_Shaders[index];
    for (u32 i = 0; i < entry.CompiledPermutations.Size(); ++i)
    {
        if (ShaderPermutation::MatchesKey(entry.CompiledPermutations[i].Key, key))
            return entry.CompiledPermutations[i].VS;
    }
    return nullptr;
}

Shader* ShaderLibrary::GetPS(u32 index, const ShaderPermutationKey& key)
{
    if (index >= m_Shaders.Size())
        return nullptr;

    ShaderSourceEntry& entry = m_Shaders[index];
    for (u32 i = 0; i < entry.CompiledPermutations.Size(); ++i)
    {
        if (ShaderPermutation::MatchesKey(entry.CompiledPermutations[i].Key, key))
            return entry.CompiledPermutations[i].PS;
    }
    return nullptr;
}

bool ShaderLibrary::GetShaders(u32 index, const ShaderPermutationKey& key, Shader*& outVS, Shader*& outPS)
{
    outVS = nullptr;
    outPS = nullptr;

    if (index >= m_Shaders.Size())
        return false;

    ShaderSourceEntry& entry = m_Shaders[index];
    for (u32 i = 0; i < entry.CompiledPermutations.Size(); ++i)
    {
        if (ShaderPermutation::MatchesKey(entry.CompiledPermutations[i].Key, key))
        {
            outVS = entry.CompiledPermutations[i].VS;
            outPS = entry.CompiledPermutations[i].PS;
            return outVS != nullptr && outPS != nullptr;
        }
    }
    return false;
}

void ShaderLibrary::CheckForChanges()
{
    if (!m_HotReloadEnabled)
        return;

    for (u32 i = 0; i < m_Shaders.Size(); ++i)
    {
        ShaderSourceEntry& entry = m_Shaders[i];
        GString fullPath = m_ShaderRootPath + "/" + entry.FilePath;
        u64 currentTimestamp = GetFileTimestamp(fullPath);

        if (currentTimestamp == 0)
            continue;

        if (entry.LastModifiedTimestamp == 0)
        {
            entry.LastModifiedTimestamp = currentTimestamp;
            continue;
        }

        if (currentTimestamp > entry.LastModifiedTimestamp)
        {
            entry.LastModifiedTimestamp = currentTimestamp;

            TArray<ShaderPermutationKey> keys;
            for (u32 j = 0; j < entry.CompiledPermutations.Size(); ++j)
            {
                keys.Add(entry.CompiledPermutations[j].Key);
            }

            if (keys.IsEmpty())
            {
                ShaderPermutationKey defaultKey{};
                keys.Add(defaultKey);
            }

            for (u32 j = 0; j < keys.Size(); ++j)
            {
                CompileShader(i, keys[j]);
            }
        }
    }
}

void ShaderLibrary::AddGlobalDefine(const GString& name, const GString& value)
{
    GString define = name + " " + value;
    m_GlobalDefines.Add(define);
}

void ShaderLibrary::RemoveGlobalDefine(const GString& name)
{
    for (u32 i = 0; i < m_GlobalDefines.Size(); ++i)
    {
        if (m_GlobalDefines[i].Find(name.c_str()) >= 0)
        {
            m_GlobalDefines.RemoveAt(i);
            return;
        }
    }
}

void ShaderLibrary::ClearGlobalDefines()
{
    m_GlobalDefines.Clear();
}

GString ShaderLibrary::GetGlobalDefinesString() const
{
    GString result;
    for (u32 i = 0; i < m_GlobalDefines.Size(); ++i)
    {
        result += "#define ";
        result += m_GlobalDefines[i];
        result += "\n";
    }
    return result;
}

}
