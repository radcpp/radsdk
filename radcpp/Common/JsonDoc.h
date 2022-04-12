#ifndef RADCPP_JSON_DOC_H
#define RADCPP_JSON_DOC_H
#pragma once

#include "radcpp/Common/Common.h"
#include "radcpp/Common/File.h"

#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"

class JsonDoc
{
public:
    JsonDoc();
    ~JsonDoc();

    bool ParseFile(const Path& filePath);
    rapidjson::Value& GetRoot() { return m_doc.GetObject(); }

private:
    rapidjson::Document m_doc;

}; // class JsonDoc

#endif // RADCPP_JSON_DOC_H