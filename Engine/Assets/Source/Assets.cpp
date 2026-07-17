#include <Assets/Assets.h>
#include <RHI/RHI_Texture.h>
#include <RHI/RHI_Device.h>
#include <Core/Types.h>
#include <Core/Memory.h>

namespace grom
{

AssetManager::AssetManager() {}
AssetManager::~AssetManager() {}

AssetHandle<TextureAsset> AssetManager::LoadTexture(const GString& path, bool sRGB,
                                                    EFormat format) {
    (void)path; (void)sRGB; (void)format;
    return AssetHandle<TextureAsset>::Invalid;
}

void AssetManager::UnloadTexture(AssetHandle<TextureAsset> handle) {
    (void)handle;
}

Texture* AssetManager::GetTextureData(AssetHandle<TextureAsset> handle) {
    (void)handle;
    return nullptr;
}

u32 AssetManager::GetTextureWidth(AssetHandle<TextureAsset> handle) {
    (void)handle;
    return 0;
}
u32 AssetManager::GetTextureHeight(AssetHandle<TextureAsset> handle) {
    (void)handle;
    return 0;
}
bool AssetManager::IsTextureLoaded(AssetHandle<TextureAsset> handle) const {
    (void)handle;
    return false;
}

AssetHandle<TextureAsset> AssetManager::LoadTextureFromMemory(const void* data, usize size,
                                                             bool sRGB, EFormat format) {
    (void)data; (void)size; (void)sRGB; (void)format;
    return AssetHandle<TextureAsset>::Invalid;
}

Texture* AssetManager::LoadTextureFromFile(const GString& path, bool sRGB,
                                          EFormat format, u32& width, u32& height, u32& mipLevels) {
    (void)path; (void)sRGB; (void)format;
    width = 0; height = 0; mipLevels = 0;
    return nullptr;
}

Texture* AssetManager::CompressTextureWithDirectXTex(const GString& inputPath,
                                                      const GString& outputPath,
                                                      EFormat format, bool sRGB) {
    (void)inputPath; (void)outputPath; (void)format; (void)sRGB;
    return nullptr;
}

Texture* AssetManager::LoadCompressedTexture(const GString& path, EFormat format) {
    (void)path; (void)format;
    return nullptr;
}

} // namespace grom