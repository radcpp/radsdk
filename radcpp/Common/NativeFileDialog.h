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

}; // class NativeFileDialog

#endif // RADCPP_NATIVEFILEDIALOG_H