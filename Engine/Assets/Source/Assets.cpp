#include <Assets/Assets.h>
#include <RHI/RHI_Texture.h>
#include <RHI/RHI_Device.h>
#include <RHI/RHI_Shader.h>
#include <RHI/RHI_Buffer.h>
#include <RHI/RHI_Pipeline.h>
#include <Core/Types.h>
#include <Core/Memory.h>
#include <cstring>
#include <system_error>

// DirectXTex includes
#define DIRECTX_TEX_PRESERVE_MICROSOFT_NAMESPACE
#include <DirectXTex.h>

namespace grom
{

AssetManager::AssetManager() : m_registry(), m_textureCache()
{
}

AssetManager::~AssetManager() {
    m_textureCache.Clear();
}

GString AssetManager::GenerateGUID() const {
    // Simple GUID generation using time-based approach
    SYSTEMTIME st;
    GetSystemTime(&st);
    char guid[64];
    snprintf(guid, sizeof(guid), "%04d%02d%02d_%02d%02d%02d%03d",
             st.wYear, st.wMonth, st.wDay,
             st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    return GString(guid);
}

GString AssetManager::GetCachePath(const GString& sourcePath, const GString& extension) const {
    // Extract filename from path
    i32 lastSlash = sourcePath.Find("/");
    i32 lastBackslash = sourcePath.Find("\\");
    i32 lastSeparator = (lastSlash > lastBackslash) ? lastSlash : lastBackslash;
    if (lastSeparator == -1) lastSeparator = 0;

    GString filename = sourcePath.SubStr(lastSeparator + 1, sourcePath.Len() - lastSeparator - 1);

    // Generate GUID for uniqueness
    GString guid = GenerateGUID();

    GString cachePath = "cache/";
    if (extension.IsEmpty()) extension = "tex";
    cachePath = cachePath + guid + "." + extension;

    return cachePath;
}

AssetHandle<TextureAsset> AssetManager::LoadTexture(const GString& path, bool sRGB,
                                                    EFormat format) {
    // Check cache first
    if (m_textureCache.Contains(path)) {
        AssetHandle<TextureAsset> cachedHandle = m_textureCache[path];
        if (m_registry.Contains(cachedHandle)) {
            return cachedHandle;
        }
    }

    u32 width = 0, height = 0, mipLevels = 0;

    // Try compressed first (cache path)
    GString cachePath = GetCachePath(path, "dds");
    Texture* tex = LoadCompressedTexture(cachePath, format);

    if (!tex) {
        // Try loading from file
        tex = LoadTextureFromFile(path, sRGB, format, width, height, mipLevels);
    }

    if (!tex) {
        return AssetHandle<TextureAsset>::Invalid;
    }

    // Create asset handle
    AssetHandle<TextureAsset> handle(GenerateGUID().GetHash(), EAssetType::Texture);

    TextureAsset asset(tex, width, height, mipLevels);
    m_registry.AddAsset<TextureAsset>(EAssetType::Texture, path);

    // Store in cache
    m_textureCache[path] = handle;

    return handle;
}

AssetHandle<TextureAsset> AssetManager::LoadTextureFromMemory(const void* data,
                                                              usize size,
                                                              bool sRGB,
                                                              EFormat format) {
    // Create texture from memory data
    GString tempPath = "memory_" + GenerateGUID().SubStr(0, 8);

    u32 width = 0, height = 0, mipLevels = 0;
    Texture* tex = LoadTextureFromFile(tempPath, sRGB, format, width, height, mipLevels);

    if (!tex) {
        return AssetHandle<TextureAsset>::Invalid;
    }

    AssetHandle<TextureAsset> handle(GenerateGUID().GetHash(), EAssetType::Texture);
    TextureAsset asset(tex, width, height, mipLevels);
    m_registry.AddAsset<TextureAsset>(EAssetType::Texture, tempPath);

    return handle;
}

void AssetManager::UnloadTexture(AssetHandle<TextureAsset> handle) {
    if (!handle.IsValid()) return;

    GString path = m_registry.GetAssetPath<TextureAsset>(handle);
    m_textureCache.Remove(path);
    m_registry.RemoveAsset<TextureAsset>(handle);
}

Texture* AssetManager::GetTextureData(AssetHandle<TextureAsset> handle) {
    TextureAsset* asset = m_registry.GetAsset<TextureAsset>(handle);
    return asset ? asset->GetTexture() : nullptr;
}

Texture* AssetManager::LoadTextureFromFile(const GString& path, bool sRGB, EFormat format,
                                           u32& width, u32& height, u32& mipLevels) {
    // Check if file exists
    FILE* file = _wfopen(GA2W(path.c_str()).c_str(), L"rb");
    if (!file) {
        return nullptr;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    usize fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate buffer
    u8* buffer = static_cast<u8*>(malloc(fileSize));
    if (!buffer) {
        fclose(file);
        return nullptr;
    }

    // Read file
    fread(buffer, 1, fileSize, file);
    fclose(file);

    // Try DirectXTex to load and possibly compress
    Texture* tex = nullptr;
    tex = CompressTextureWithDirectXTex(path, path, format, sRGB);

    if (!tex) {
        free(buffer);
        return nullptr;
    }

    // Get texture info
    Texture* texture = static_cast<Texture*>(tex);
    width = texture->GetDesc().Width;
    height = texture->GetDesc().Height;
    mipLevels = texture->GetDesc().MipLevels;

    return texture;
}

Texture* AssetManager::CompressTextureWithDirectXTex(const GString& inputPath,
                                                      const GString& outputPath,
                                                      EFormat format, bool sRGB) {
    // Convert format to DXGI_FORMAT
    DXGI_FORMAT dxgiFormat = DXGI_FORMAT_UNKNOWN;
    switch (format) {
    case EFormat::R8G8B8A8_UNORM: dxgiFormat = sRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM; break;
    case EFormat::R8G8B8A8_SRGB: dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; break;
    case EFormat::R16G16B16A16_FLOAT: dxgiFormat = DXGI_FORMAT_R16G16B16A16_FLOAT; break;
    case EFormat::R32G32B32A32_FLOAT: dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
    default: return nullptr;
    }

    // Load texture using DirectXTex
    DirectX::ScratchImage scratchImage;
    HRESULT hr = DirectX::LoadFromWICFile(GA2W(inputPath.c_str()).c_str(),
                                           DirectX::WIC_FLAGS_IGNORE_SRGB,
                                           nullptr, scratchImage);
    if (FAILED(hr)) {
        return nullptr;
    }

    // Convert to target format
    DirectX::ScratchImage convertedImage;
    hr = DirectX::Convert(*scratchImage.GetImage(0, 0, 0), dxgiFormat, false, 0.0f, convertedImage);
    if (FAILED(hr)) {
        return nullptr;
    }

    // Compress to BC formats for better VRAM usage
    DirectX::ScratchImage compressedImage;
    hr = DirectX::Compress(convertedImage.GetImage(0, 0, 0)->width,
                           convertedImage.GetImage(0, 0, 0)->height,
                           1,
                           DXGI_FORMAT_BC3_UNORM,
                           DirectX::TEX_COMPRESS_BC1_RECOVERY | DirectX::TEX_COMPRESS_SRGB_OUT,
                           0.5f,
                           convertedImage.GetImage(0, 0, 0)->pixels, compressedImage);

    if (SUCCEEDED(hr)) {
        // Create texture from compressed data
        TextureDesc desc;
        const DirectX::Image* img = compressedImage.GetImage(0, 0, 0);
        desc.Width = img->width;
        desc.Height = img->height;
        desc.Format = format;
        desc.MipLevels = 1;
        desc.Depth = 1;
        desc.ArraySize = 1;

        // Create D3D11 texture with compressed data
        ERenderAPI api = GROM_RHI_D3D11 ? ERenderAPI::D3D11 : ERenderAPI::Vulkan;

        Texture* texture = Texture::Create(desc, api);
        if (texture) {
            texture->Upload(compressedImage.GetPixels(), 0);
            return texture;
        }
    }

    return nullptr;
}

Texture* AssetManager::LoadCompressedTexture(const GString& path, EFormat format) {
    // Load already compressed texture file (DDS, etc.)
    FILE* file = _wfopen(GA2W(path.c_str()).c_str(), L"rb");
    if (!file) return nullptr;

    fseek(file, 0, SEEK_END);
    usize fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    u8* buffer = static_cast<u8*>(malloc(fileSize));
    if (!buffer) {
        fclose(file);
        return nullptr;
    }

    fread(buffer, 1, fileSize, file);
    fclose(file);

    // Create texture from compressed data
    TextureDesc desc;
    desc.Width = 0;
    desc.Height = 0;
    desc.Format = format;
    desc.MipLevels = 1;
    desc.Depth = 1;
    desc.ArraySize = 1;

    ERenderAPI api = GROM_RHI_D3D11 ? ERenderAPI::D3D11 : ERenderAPI::Vulkan;
    Texture* texture = Texture::Create(desc, api);

    if (texture) {
        texture->Upload(buffer, 0);
        free(buffer);
        return texture;
    }

    free(buffer);
    return nullptr;
}

bool AssetManager::IsTextureLoaded(AssetHandle<TextureAsset> handle) const {
    TextureAsset* asset = m_registry.GetAsset<TextureAsset>(handle);
    return asset ? asset->IsLoaded() : false;
}

u32 AssetManager::GetTextureWidth(AssetHandle<TextureAsset> handle) {
    TextureAsset* asset = m_registry.GetAsset<TextureAsset>(handle);
    return asset ? asset->GetWidth() : 0;
}

u32 AssetManager::GetTextureHeight(AssetHandle<TextureAsset> handle) {
    TextureAsset* asset = m_registry.GetAsset<TextureAsset>(handle);
    return asset ? asset->GetHeight() : 0;
}

template<typename T>
AssetHandle<T> AssetRegistry::AddAsset(EAssetType type, const GString& path) {
    u64 id = m_nextId++;
    AssetEntry* entry = static_cast<AssetEntry*>(malloc(sizeof(AssetEntry) + sizeof(T)));
    if (!entry) return AssetHandle<T>::Invalid;

    entry->type = type;
    entry->path = path;
    entry->typeName = typeid(T).name();
    // In a real implementation, we would copy the actual asset data here

    m_assets.Add(entry);
    m_handleToEntry[id] = entry;
    m_pathToHandle[path] = AssetHandle<T>(id, type);

    return AssetHandle<T>(id, type);
}

template<typename T>
T* AssetRegistry::GetAsset(AssetHandle<T> handle) {
    auto it = m_handleToEntry.Find(handle.GetId());
    if (it == m_handleToEntry.end()) return nullptr;

    AssetEntry* entry = it->second;
    if (entry->type != handle.GetType()) return nullptr;

    return reinterpret_cast<T*>(entry->data);
}

template<typename T>
void AssetRegistry::RemoveAsset(AssetHandle<T> handle) {
    auto it = m_handleToEntry.Find(handle.GetId());
    if (it == m_handleToEntry.end()) return;

    AssetEntry* entry = it->second;
    m_assets.RemoveSwap(it->first);
    m_handleToEntry.Remove(handle.GetId());

    if (m_pathToHandle.Contains(entry->path)) {
        GString path = entry->path;
        m_pathToHandle.Remove(path);
    }

    free(entry);
}

template<typename T>
bool AssetRegistry::Contains(AssetHandle<T> handle) const {
    return m_handleToEntry.Contains(handle.GetId());
}

template<typename T>
GString AssetRegistry::GetAssetPath(AssetHandle<T> handle) const {
    auto it = m_handleToEntry.Find(handle.GetId());
    if (it == m_handleToEntry.end()) return "";
    return it->second->path;
}

void AssetRegistry::Clear() {
    for (AssetEntry* entry : m_assets) {
        free(entry);
    }
    m_assets.Clear();
    m_handleToEntry.Clear();
    m_pathToHandle.Clear();
}

usize AssetRegistry::GetAssetCount() const {
    return m_assets.Size();
}

} // namespace grom
