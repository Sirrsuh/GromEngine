#pragma once
#include <Core/Container.h>
#include <String.h>

namespace grom
{

class IFileSystem
{
public:
    virtual ~IFileSystem() {}

    virtual bool Exists(const GString& path) const = 0;
    virtual usize GetSize(const GString& path) const = 0;
    virtual GString ReadText(const GString& path) const = 0;
    virtual bool ReadBinary(const GString& path, TArray<u8>& outData) const = 0;
    virtual bool WriteText(const GString& path, const GString& content) const = 0;
    virtual bool WriteBinary(const GString& path, const u8* data, usize size) const = 0;
    virtual bool Delete(const GString& path) const = 0;
    virtual bool Rename(const GString& from, const GString& to) const = 0;

    virtual bool CreateDirectory(const GString& path) const = 0;
    virtual bool DeleteDirectory(const GString& path, bool recursive = false) const = 0;
    virtual bool GetChildren(const GString& path, TArray<GString>& outEntries) const = 0;
    virtual bool IsDirectory(const GString& path) const = 0;

    virtual TArray<GString> FindFiles(const GString& path, const GString& pattern = "*") const = 0;
    virtual GString GetWorkingDirectory() const = 0;
    virtual bool SetWorkingDirectory(const GString& path) = 0;
    virtual GString GetRealPath(const GString& path) const = 0;
};

class NativeFileSystem : public IFileSystem
{
public:
    NativeFileSystem() = default;
    ~NativeFileSystem() override = default;

    bool Exists(const GString& path) const override;
    usize GetSize(const GString& path) const override;
    GString ReadText(const GString& path) const override;
    bool ReadBinary(const GString& path, TArray<u8>& outData) const override;
    bool WriteText(const GString& path, const GString& content) const override;
    bool WriteBinary(const GString& path, const u8* data, usize size) const override;
    bool Delete(const GString& path) const override;
    bool Rename(const GString& from, const GString& to) const override;

    bool CreateDirectory(const GString& path) const override;
    bool DeleteDirectory(const GString& path, bool recursive = false) const override;
    bool GetChildren(const GString& path, TArray<GString>& outEntries) const override;
    bool IsDirectory(const GString& path) const override;

    TArray<GString> FindFiles(const GString& path, const GString& pattern = "*") const override;
    GString GetWorkingDirectory() const override;
    bool SetWorkingDirectory(const GString& path) override;
    GString GetRealPath(const GString& path) const override;
};

class PackageFileSystem : public IFileSystem
{
public:
    PackageFileSystem() = default;
    ~PackageFileSystem() override = default;

    bool Open(const GString& packagePath);
    void Close();

    bool Exists(const GString& path) const override;
    usize GetSize(const GString& path) const override;
    GString ReadText(const GString& path) const override;
    bool ReadBinary(const GString& path, TArray<u8>& outData) const override;
    bool WriteText(const GString& path, const GString& content) const override;
    bool WriteBinary(const GString& path, const u8* data, usize size) const override;
    bool Delete(const GString& path) const override;
    bool Rename(const GString& from, const GString& to) const override;

    bool CreateDirectory(const GString& path) const override;
    bool DeleteDirectory(const GString& path, bool recursive = false) const override;
    bool GetChildren(const GString& path, TArray<GString>& outEntries) const override;
    bool IsDirectory(const GString& path) const override;

    TArray<GString> FindFiles(const GString& path, const GString& pattern = "*") const override;
    GString GetWorkingDirectory() const override;
    bool SetWorkingDirectory(const GString& path) override;
    GString GetRealPath(const GString& path) const override;

private:
    GString m_packagePath;
    IFileSystem* m_baseFS = nullptr;
    GHashMap<GString, usize> m_fileOffsets;
    GHashMap<GString, usize> m_fileSizes;
    TArray<u8> m_fileData;
};

extern IFileSystem& GetFileSystem();
inline void SetFileSystem(IFileSystem& fs) { delete &GetFileSystem(); }

} // namespace grom
