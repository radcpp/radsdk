#ifndef RADCPP_NATIVEFILEDIALOG_H
#define RADCPP_NATIVEFILEDIALOG_H

#include "Common.h"
#include "File.h"
#include "ArrayRef.h"

class NativeFileDialog
{
public:
    enum Result
    {
        ResultError,
        ResultOkay,
        ResultCancel,
    };

    struct FilterItem
    {
        const char* name;
        const char* spec;
    };

    NativeFileDialog();
    ~NativeFileDialog();

    bool Init();
    void Quit();

    Result OpenFileDialog(Path& path, ArrayRef<FilterItem> filters, const Path* defaultPath = nullptr);
    Result OpenFileDialogMultiSelect(std::vector<Path>& paths, ArrayRef<FilterItem> filters, const Path* defaultPath = nullptr);
    Result SaveFileDialog(Path& path, ArrayRef<FilterItem> filters, const Path* defaultPath = nullptr, const Path* defaultName = nullptr);
    Result PickFolder(Path& path, const Path* defaultPath = nullptr);

    const char* GetError() const;

}; // class NativeFileDialog

#endif // RADCPP_NATIVEFILEDIALOG_H