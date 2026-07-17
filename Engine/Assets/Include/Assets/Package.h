#pragma once
#include <Core/Types.h>
#include <Core/Container.h>

namespace grom::Package
{

enum class ECompressionType : u8
{
    None = 0,
    LZ4 = 1,
    Zstd = 2,
};

enum class EEncryptionType : u8
{
    None = 0,
    AES256 = 1,
    ChaCha20 = 2,
};

struct PackageHeader
{
    u32 Magic = 0x47524F4D; // 'GROM'
    u32 Version = 1;
    u64 IndexOffset = 0;
    u64 IndexSize = 0;
    u32 EntryCount = 0;
    u8 Compression = 0;
    u8 Encryption = 0;
    u8 KeyHash[32] = {};
    u8 Reserved[64] = {};
};

struct PackageEntry
{
    u64 Hash = 0;
    u64 Offset = 0;
    u64 CompressedSize = 0;
    u64 UncompressedSize = 0;
    u32 Flags = 0;
    u32 CompressionType = 0;
    u32 CRC32 = 0;
};

class PackageReader
{
public:
    PackageReader() = default;
    ~PackageReader();

    bool Open(const GString& path, const GString& password = "");
    void Close();

    bool HasEntry(const GString& path) const;
    bool HasEntry(u64 hash) const;

    bool ReadEntry(const GString& path, TArray<u8>& outData) const;
    bool ReadEntry(u64 hash, TArray<u8>& outData) const;
    GString ReadEntryText(const GString& path) const;

    void GetAllPaths(TArray<GString>& outPaths) const;

    bool IsOpen() const { return m_Open; }

private:
    bool LoadIndex();

    // Stub implementation
    bool m_Open = false;
};

class PackageWriter
{
public:
    PackageWriter() = default;
    ~PackageWriter();

    bool Create(const GString& path, const GString& password = "");
    void Close();

    bool AddFile(const GString& archivePath, const GString& diskPath);
    bool AddFileFromMemory(const GString& archivePath, const u8* data, u64 size);
    bool AddDirectory(const GString& archivePath, const GString& diskPath);

    bool Finalize();

    u64 GetTotalSize() const { return 0; }
    u32 GetEntryCount() const { return 0; }

private:
    // Stub implementation
    bool m_Finalized = false;
};

namespace Package
{
    bool Create(const GString& outputPath, const GString& sourceDir, const GString& password = "");
    bool Extract(const GString& packagePath, const GString& outputDir, const GString& password = "");
    bool List(const GString& packagePath, TArray<GString>& outPaths, const GString& password = "");
}

} // namespace grom::Package