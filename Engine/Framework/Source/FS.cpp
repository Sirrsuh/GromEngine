#include <Framework/FS.h>
#include <Core/Memory.h>
#include <Core/Container.h>
#include <Core/Types.h>
#include <Core/Assert.h>
#include <cstring>
#include <direct.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <Windows.h>
#endif

namespace grom
{

static IFileSystem* g_FileSystemInstance = nullptr;
IFileSystem& GetFileSystem() { return *g_FileSystemInstance; }
void SetFileSystem(IFileSystem& fs) {
    delete g_FileSystemInstance;
    g_FileSystemInstance = &fs;
}

bool NativeFileSystem::Exists(const GString& path) const {
#ifdef _WIN32
    struct _stat st;
    return _stat(GA2W(path.c_str()).c_str(), &st) == 0;
#endif
    return false;
}

usize NativeFileSystem::GetSize(const GString& path) const {
#ifdef _WIN32
    struct _stat st;
    if (_stat(GA2W(path.c_str()).c_str(), &st) == 0) {
        return st.st_size;
    }
#endif
    return 0;
}

GString NativeFileSystem::ReadText(const GString& path) const {
    FILE* file = _wfopen(GA2W(path.c_str()).c_str(), L"r");
    if (!file) return GString();

    fseek(file, 0, SEEK_END);
    usize size = ftell(file);
    fseek(file, 0, SEEK_SET);

    GString result;
    result.Reserve(size + 1);

    char* buffer = static_cast<char*>(malloc(size + 1));
    if (buffer) {
        fread(buffer, 1, size, file);
        buffer[size] = '\0';
        result.Append(buffer);
        free(buffer);
    }

    fclose(file);
    return result;
}

bool NativeFileSystem::ReadBinary(const GString& path, TArray<u8>& outData) const {
    FILE* file = _wfopen(GA2W(path.c_str()).c_str(), L"rb");
    if (!file) return false;

    fseek(file, 0, SEEK_END);
    usize size = ftell(file);
    fseek(file, 0, SEEK_SET);

    outData.Resize(size);
    usize read = fread(outData.Data(), 1, size, file);
    fclose(file);

    return read == size;
}

bool NativeFileSystem::WriteText(const GString& path, const GString& content) const {
    FILE* file = _wfopen(GA2W(path.c_str()).c_str(), L"w");
    if (!file) return false;

    usize len = content.Len();
    fwrite(content.c_str(), 1, len, file);
    fclose(file);
    return true;
}

bool NativeFileSystem::WriteBinary(const GString& path, const u8* data, usize size) const {
    FILE* file = _wfopen(GA2W(path.c_str()).c_str(), L"wb");
    if (!file) return false;

    usize written = fwrite(data, 1, size, file);
    fclose(file);
    return written == size;
}

bool NativeFileSystem::Delete(const GString& path) const {
#ifdef _WIN32
    return _wremove(GA2W(path.c_str()).c_str()) == 0;
#endif
    return false;
}

bool NativeFileSystem::Rename(const GString& from, const GString& to) const {
#ifdef _WIN32
    return _wrename(GA2W(from.c_str()).c_str(), GA2W(to.c_str()).c_str()) == 0;
#endif
    return false;
}

bool NativeFileSystem::CreateDirectory(const GString& path) const {
#ifdef _WIN32
    return _wmkdir(GA2W(path.c_str()).c_str()) == 0 || errno == EEXIST;
#endif
    return false;
}

bool NativeFileSystem::DeleteDirectory(const GString& path, bool recursive) const {
#ifdef _WIN32
    return _wrmdir(GA2W(path.c_str()).c_str()) == 0 || errno == ENOTEMPTY;
#endif
    return false;
}

bool NativeFileSystem::GetChildren(const GString& path, TArray<GString>& outEntries) const {
#ifdef _WIN32
    GString searchPath = path + "/*";
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(GA2W(searchPath.c_str()).c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) return false;

    bool hasMore = true;
    while (hasMore) {
        wchar_t wname[MAX_PATH];
        wchar_t wrelname[MAX_PATH];
        wcscpy(wname, findData.cFileName);
        wcscpy(wrelname, L"");

        if (wcscmp(findData.cFileName, L"..") == 0 ||
            wcscmp(findData.cFileName, L".") == 0) {
            // Skip parent and current directories
        } else {
            char name[MAX_PATH];
            WideCharToMultiByte(CP_UTF8, 0, findData.cFileName, -1, name, MAX_PATH, NULL, NULL);
            outEntries.Add(GString(name));
        }

        if (!FindNextFileW(hFind, &findData)) {
            hasMore = (GetLastError() == ERROR_NO_MORE_FILES);
        }
    }
    FindClose(hFind);
#endif
    return true;
}

bool NativeFileSystem::IsDirectory(const GString& path) const {
#ifdef _WIN32
    struct _stat st;
    if (_stat(GA2W(path.c_str()).c_str(), &st) == 0) {
        return (st.st_mode & _S_IFDIR) != 0;
    }
#endif
    return false;
}

TArray<GString> NativeFileSystem::FindFiles(const GString& path, const GString& pattern) const {
    TArray<GString> results;
#ifdef _WIN32
    GString searchPath = path + "/" + pattern;
    WIN32_FIND_DATAW findData;
    HANDLE hFind = FindFirstFileW(GA2W(searchPath.c_str()).c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE) return results;

    bool hasMore = true;
    while (hasMore) {
        wchar_t wname[MAX_PATH];
        wcscpy(wname, findData.cFileName);

        char name[MAX_PATH];
        WideCharToMultiByte(CP_UTF8, 0, findData.cFileName, -1, name, MAX_PATH, NULL, NULL);
        results.Add(GString(name));

        if (!FindNextFileW(hFind, &findData)) {
            hasMore = (GetLastError() == ERROR_NO_MORE_FILES);
        }
    }
    FindClose(hFind);
#endif
    return results;
}

GString NativeFileSystem::GetWorkingDirectory() const {
#ifdef _WIN32
    wchar_t buffer[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, buffer);
    char result[MAX_PATH];
    WideCharToMultiByte(CP_UTF8, 0, buffer, -1, result, MAX_PATH, NULL, NULL);
    return GString(result);
#endif
    return GString();
}

bool NativeFileSystem::SetWorkingDirectory(const GString& path) {
#ifdef _WIN32
    return SetCurrentDirectoryW(GA2W(path.c_str()).c_str()) != 0;
#endif
    return false;
}

GString NativeFileSystem::GetRealPath(const GString& path) const {
    return path;
}

} // namespace grom
