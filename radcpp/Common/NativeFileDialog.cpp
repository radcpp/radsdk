#include "NativeFileDialog.h"

#include "nfd.h"

static_assert(NativeFileDialog::ResultError == NFD_ERROR);
static_assert(NativeFileDialog::ResultOkay == NFD_OKAY);
static_assert(NativeFileDialog::ResultCancel == NFD_CANCEL);

NativeFileDialog::NativeFileDialog()
{
}

NativeFileDialog::~NativeFileDialog()
{
}

bool NativeFileDialog::Init()
{
    if (NFD_Init() == NFD_OKAY)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void NativeFileDialog::Quit()
{
    NFD_Quit();
}

NativeFileDialog::Result NativeFileDialog::OpenFileDialog(Path& path, ArrayRef<FilterItem> filters, const Path* defaultPath)
{
    nfdu8char_t* outPath = nullptr;
    std::vector<nfdfilteritem_t> filterItems(filters.size());
    for (uint32_t i = 0; i < filters.size(); ++i)
    {
        filterItems[i].name = filters[i].name;
        filterItems[i].spec = filters[i].spec;
    }
    nfdresult_t result = NFD_OpenDialogU8(&outPath,
        filterItems.data(), static_cast<nfdfiltersize_t>(filterItems.size()),
        defaultPath ? (nfdu8char_t*)defaultPath->u8string().c_str() : nullptr);
    if (result == NFD_OKAY)
    {
        path = (char8_t*)outPath;
        NFD_FreePathU8(outPath);
    }
    return static_cast<Result>(result);
}

NativeFileDialog::Result NativeFileDialog::OpenFileDialogMultiSelect(std::vector<Path>& paths, ArrayRef<FilterItem> filters, const Path* defaultPath)
{
    const nfdpathset_t* outPaths = nullptr;
    std::vector<nfdfilteritem_t> filterItems(filters.size());
    for (uint32_t i = 0; i < filters.size(); ++i)
    {
        filterItems[i].name = filters[i].name;
        filterItems[i].spec = filters[i].spec;
    }
    nfdresult_t result = NFD_OpenDialogMultipleU8(&outPaths,
        filterItems.data(), static_cast<nfdfiltersize_t>(filterItems.size()),
        defaultPath ? (nfdu8char_t*)defaultPath->u8string().c_str() : nullptr);
    if (result == NFD_OKAY)
    {
        nfdpathsetsize_t pathCount = 0;
        if (NFD_PathSet_GetCount(outPaths, &pathCount) == NFD_OKAY)
        {
            paths.reserve(pathCount);
            for (nfdpathsetsize_t i = 0; i < pathCount; ++i)
            {
                nfdu8char_t* outPath = nullptr;
                if (NFD_PathSet_GetPathU8(outPaths, i, &outPath) == NFD_OKAY)
                {
                    paths.push_back((char8_t*)outPath);
                    NFD_FreePathU8(outPath);
                }
            }
        }
        NFD_PathSet_Free(outPaths);
    }
    return static_cast<Result>(result);
}

NativeFileDialog::Result NativeFileDialog::SaveFileDialog(Path& path, ArrayRef<FilterItem> filters, const Path* defaultPath, const Path* defaultName)
{
    nfdu8char_t* outPath = nullptr;
    std::vector<nfdfilteritem_t> filterItems(filters.size());
    for (uint32_t i = 0; i < filters.size(); ++i)
    {
        filterItems[i].name = filters[i].name;
        filterItems[i].spec = filters[i].spec;
    }
    nfdresult_t result = NFD_SaveDialogU8(&outPath,
        filterItems.data(), static_cast<nfdfiltersize_t>(filterItems.size()),
        defaultPath ? (nfdu8char_t*)defaultPath->u8string().c_str() : nullptr,
        defaultName ? (nfdu8char_t*)defaultName->u8string().c_str() : nullptr);
    if (result == NFD_OKAY)
    {
        path = (char8_t*)outPath;
        NFD_FreePathU8(outPath);
    }
    return static_cast<Result>(result);
}

NativeFileDialog::Result NativeFileDialog::PickFolder(Path& path, const Path* defaultPath)
{
    nfdu8char_t* outPath = nullptr;
    nfdresult_t result = NFD_PickFolderU8(&outPath,
        defaultPath ? (nfdu8char_t*)defaultPath->u8string().c_str() : nullptr);
    if (result == NFD_OKAY)
    {
        path = (char8_t*)outPath;
        NFD_FreePathU8(outPath);
    }
    return static_cast<Result>(result);
}

const char* NativeFileDialog::GetError() const
{
    return NFD_GetError();
}
