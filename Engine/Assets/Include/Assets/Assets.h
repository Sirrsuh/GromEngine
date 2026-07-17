#pragma once
#include <Core/Container.h>
#include <RHI/RHI_Texture.h>
#include <String.h>

namespace grom
{

enum class EAssetType
{
    None,
    Texture,
    Material,
    Mesh,
    Shader,
    Animation,
    Audio,
    Count
};

template<typename T>
class AssetHandle
{
public:
    AssetHandle() : m_id(0), m_type(EAssetType::None) {}
    AssetHandle(u64 id, EAssetType type) : m_id(id), m_type(type) {}

    bool IsValid() const { return m_id != 0; }
    u64 GetId() const { return m_id; }
    EAssetType GetType() const { return m_type; }

    bool operator==(const AssetHandle& other) const { return m_id == other.m_id; }
    bool operator!=(const AssetHandle& other) const { return m_id != other.m_id; }
    bool operator<(const AssetHandle& other) const { return m_id < other.m_id; }

    static const AssetHandle<T> Invalid;

private:
    u64 m_id;
    EAssetType m_type;
};

template<typename T>
const AssetHandle<T> AssetHandle<T>::Invalid = AssetHandle<T>(0, EAssetType::None);

class TextureAsset
{
public:
    TextureAsset() : m_texture(nullptr), m_width(0), m_height(0), m_mipLevels(0) {}
    TextureAsset(Texture* texture, u32 width, u32 height, u32 mipLevels)
        : m_texture(texture), m_width(width), m_height(height), m_mipLevels(mipLevels) {}

    Texture* GetTexture() const { return m_texture; }
    u32 GetWidth() const { return m_width; }
    u32 GetHeight() const { return m_height; }
    u32 GetMipLevels() const { return m_mipLevels; }

    bool IsLoaded() const { return m_texture != nullptr; }

private:
    Texture* m_texture;
    u32 m_width;
    u32 m_height;
    u32 m_mipLevels;
};

class AssetManager
{
public:
    AssetManager();
    ~AssetManager();

    AssetHandle<TextureAsset> LoadTexture(const GString& path, bool sRGB = true,
                                         EFormat format = EFormat::R8G8B8A8_UNORM);
    void UnloadTexture(AssetHandle<TextureAsset> handle);

    Texture* GetTextureData(AssetHandle<TextureAsset> handle);
    u32 GetTextureWidth(AssetHandle<TextureAsset> handle);
    u32 GetTextureHeight(AssetHandle<TextureAsset> handle);
    bool IsTextureLoaded(AssetHandle<TextureAsset> handle) const;

    AssetHandle<TextureAsset> LoadTextureFromMemory(const void* data, usize size,
                                                   bool sRGB = true,
                                                   EFormat format = EFormat::R8G8B8A8_UNORM);

private:
    GString GenerateGUID() const;
    GString GetCachePath(const GString& sourcePath, const GString& extension = "") const;

    Texture* LoadTextureFromFile(const GString& path, bool sRGB,
                                EFormat format, u32& width, u32& height, u32& mipLevels);
    Texture* CompressTextureWithDirectXTex(const GString& inputPath, const GString& outputPath,
                                           EFormat format, bool sRGB);
    Texture* LoadCompressedTexture(const GString& path, EFormat format);

    // Note: For production, use proper hash map. Using simple stub for now.
    struct TextureCacheEntry {
        GString path;
        AssetHandle<TextureAsset> handle;
    };
    TArray<TextureCacheEntry> m_textureCache;
};

} // namespace grom