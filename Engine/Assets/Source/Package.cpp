#include "Assets/Package.h"

namespace grom::Package
{

// ============================================================================
// PackageReader Implementation
// ============================================================================

PackageReader::~PackageReader()
{
    Close();
}

bool PackageReader::Open(const GString& path, const GString& password)
{
    (void)path; (void)password;
    return false;
}

void PackageReader::Close()
{
}

bool PackageReader::HasEntry(const GString& path) const
{
    (void)path;
    return false;
}

bool PackageReader::HasEntry(u64 hash) const
{
    (void)hash;
    return false;
}

bool PackageReader::ReadEntry(const GString& path, TArray<u8>& outData) const
{
    (void)path; (void)outData;
    return false;
}

bool PackageReader::ReadEntry(u64 hash, TArray<u8>& outData) const
{
    (void)hash; (void)outData;
    return false;
}

GString PackageReader::ReadEntryText(const GString& path) const
{
    (void)path;
    return GString();
}

void PackageReader::GetAllPaths(TArray<GString>& outPaths) const
{
    (void)outPaths;
}

bool PackageReader::LoadIndex()
{
    return true;
}

// ============================================================================
// PackageWriter Implementation
// ============================================================================

PackageWriter::~PackageWriter()
{
    Close();
}

bool PackageWriter::Create(const GString& path, const GString& password)
{
    (void)path; (void)password;
    return false;
}

void PackageWriter::Close()
{
}

bool PackageWriter::AddFile(const GString& archivePath, const GString& diskPath)
{
    (void)archivePath; (void)diskPath;
    return false;
}

bool PackageWriter::AddFileFromMemory(const GString& archivePath, const u8* data, u64 size)
{
    (void)archivePath; (void)data; (void)size;
    return false;
}

bool PackageWriter::AddDirectory(const GString& archivePath, const GString& diskPath)
{
    (void)archivePath; (void)diskPath;
    return false;
}

bool PackageWriter::Finalize()
{
    return false;
}

// High-level API
bool Package::Create(const GString& outputPath, const GString& sourceDir, const GString& password)
{
    (void)outputPath; (void)sourceDir; (void)password;
    return false;
}

bool Package::Extract(const GString& packagePath, const GString& outputDir, const GString& password)
{
    (void)packagePath; (void)outputDir; (void)password;
    return false;
}

bool Package::List(const GString& packagePath, TArray<GString>& outPaths, const GString& password)
{
    (void)packagePath; (void)outPaths; (void)password;
    return false;
}

} // namespace grom::Package