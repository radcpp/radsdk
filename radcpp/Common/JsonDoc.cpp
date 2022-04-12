#include "radcpp/Common/JsonDoc.h"
#include "radcpp/Common/Containers.h"
#include "radcpp/Common/Log.h"

#include "rapidjson/error/error.h"
#include "rapidjson/error/en.h"

JsonDoc::JsonDoc()
{
}

JsonDoc::~JsonDoc()
{
}

bool JsonDoc::ParseFile(const Path& filePath)
{
    std::string jsonString = File::ReadAll(filePath);
    if (m_doc.Parse<
        rapidjson::ParseFlag::kParseCommentsFlag |
        rapidjson::ParseFlag::kParseTrailingCommasFlag |
        rapidjson::ParseFlag::kParseNanAndInfFlag>
        (jsonString.c_str()).HasParseError())
    {
        const char* errorString = rapidjson::GetParseError_En(m_doc.GetParseError());
        size_t errorOffset = m_doc.GetErrorOffset();
        size_t errorLineNo = std::count(jsonString.begin(), jsonString.begin() + errorOffset, '\n') + 1;
        LogPrint("Global", LogLevel::Error, "JSON parse error in file %s, around line %u: %s",
            (const char*)filePath.u8string().c_str(), errorLineNo, errorString);
        return false;
    }
    else
    {
        return true;
    }
}
