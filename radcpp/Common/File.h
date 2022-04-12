#ifndef RADCPP_FILE_H
#define RADCPP_FILE_H
#pragma once

#include "radcpp/Common/Common.h"
#include "radcpp/Common/String.h"
#include <filesystem>

using Path = std::filesystem::path;
using DirectoryEntry = std::filesystem::directory_entry;
using DirectoryIterator = std::filesystem::directory_iterator;
using FileTime = std::filesystem::file_time_type;
using FileStatus = std::filesystem::file_status;
using FileType = std::filesystem::file_type;
using FilePermission = std::filesystem::perms;
using FileStatus = std::filesystem::file_status;

enum FileOpenFlagBits : uint32_t
{
    FileOpenRead = 0x00000001,
    FileOpenWrite = 0x00000002,
    FileOpenAppend = 0x00000004,
    FileOpenBinary = 0x00000008,
};
using FileOpenFlags = uint32_t;

class File
{
public:
    File();
    ~File();

    bool Open(const Path& filePath, FileOpenFlags flags);
    void Close();
    bool IsOpen();
    void Flush();

    size_t Read(void* buffer, size_t sizeInBytes, size_t elementCount = 1);
    size_t ReadLine(void* buffer, size_t bufferSize);
    size_t Write(const void* buffer, size_t sizeInBytes, size_t count = 1);

    int32_t GetChar();

    // @whence: any of RW_SEEK_SET, RW_SEEK_CUR, RW_SEEK_END;
    int64_t Seek(int64_t offset, int whence);
    int64_t Rseek(int64_t offset);
    void Rewind();
    void FastForward();

    int64_t Size();
    int64_t Tell();

    static bool Exists(const Path& path);
    static std::string ReadAll(const Path& path);
    static std::vector<std::string> ReadLines(const Path& path);

    const Path& GetPath() const { return m_path; }

private:
    Path m_path;
    std::FILE* m_handle = nullptr;

}; // class File

// C++17 FileSystem
namespace FileSystem
{
    // returns or sets the current working directory
    Path GetCurrentPath();
    void SetCurrentPath(const Path& path);
    Path GetTempDirectory();

    Path GetAbsolutePath(const Path& path);
    Path GetCanonicalPath(const Path& path);
    Path GetRelativePath(const Path& path, const Path& base = GetCurrentPath());

    using CopyOptions = std::filesystem::copy_options;
    void Copy(const Path& from, const Path& to, CopyOptions options = CopyOptions::none);

    bool CreateDirectories(const Path& path);

    bool Exists(const Path& path);
    void Rename(const Path& oldPath, const Path& newPath);
    // Changes the size of the regular file named by p as if by POSIX truncate:
    // if the file size was previously larger than new_size, the remainder of the file is discarded.
    // If the file was previously smaller than new_size, the file size is increased and the new area appears as if zero-filled.
    void ResizeFile(const Path& path, uint64_t new_size);

    FileTime GetLastWriteTime(const Path& path);

    std::vector<Path> GetRootDirectories();

    // return true if the path refers to block device
    bool IsBlockFile(const Path& path);
    // return true if the path refers to character device
    bool IsCharacterFile(const Path& path);
    bool IsDirectory(const Path& path);
    bool IsEmpty(const Path& path);
    // named pipe
    bool IsFIFO(const Path& path);
    bool IsOther(const Path& path);
    bool IsRegularFile(const Path& path);
    // return true if the path refers to a named IPC socket
    bool IsSocket(const Path& path);
    bool IsSymbolLink(const Path& path);
    bool IsStatusKnown(FileStatus s);

    FileStatus GetStatus(const Path& path) ;

    uint32_t GetPathElementCount(const Path& path);
    std::string GetPathTimeString(const Path& path);

    std::string GetProcessName();

} // namesapce FileSystem

#endif // RADCPP_FILE_H