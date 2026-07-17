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
    return AssetHandle<TextureAsset>::Invalid;
}

void AssetManager::UnloadTexture(AssetHandle<TextureAsset> handle) {}

Texture* AssetManager::GetTextureData(AssetHandle<TextureAsset> handle) {
    return nullptr;
}

u32 AssetManager::GetTextureWidth(AssetHandle<TextureAsset> handle) { return 0; }
u32 AssetManager::GetTextureHeight(AssetHandle<TextureAsset> handle) { return 0; }
bool AssetManager::IsTextureLoaded(AssetHandle<TextureAsset> handle) const { return false; }

AssetHandle<TextureAsset> AssetManager::LoadTextureFromMemory(const void* data, usize size,
                                                              bool sRGB, EFormat format) {
    return AssetHandle<TextureAsset>::Invalid;
}

Texture* AssetManager::LoadTextureFromFile(const GString& path, bool sRGB, EFormat format,
                                           u32& width, u32& height, u32& mipLevels) {
    return nullptr;
}

Texture* AssetManager::CompressTextureWithDirectXTex(const GString& inputPath,
                                                      const GString& outputPath,
                                                      EFormat format, bool sRGB) {
    return nullptr;
}

Texture* AssetManager::LoadCompressedTexture(const GString& path, EFormat format) {
    return nullptr;
}

} // namespace grom