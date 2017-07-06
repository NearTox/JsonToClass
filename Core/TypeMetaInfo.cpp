//
// Copyright 2017: Ernesto Ramirez<NearTox@outlook.com>
// TODO: Put a Licence Here
// Meanwhile... I wrote this code in hope that someone improve this code
//

#include "TypeMetaInfo.hpp"

#include <NearTox/Base.hpp>
#include <NearTox/PathDir.hpp>
#include <NearTox/Internet/UrlAnalizer.hpp>

#include <fmt/format.h>

namespace NearTox {

  Config_Helper::Config_Helper(const BasicJSON &json) {
    const BasicJSON *config = json.at("--config");
    if(config && config->GetType() == JS_Object) {
      const BasicJSON *nameFormat = config->at("name-format");
      const BasicJSON *prefix = config->at("prefix");
      const BasicJSON *postfix = config->at("postfix");
      const BasicJSON *classPrefix = config->at("class-prefix");
      const BasicJSON *classPostfix = config->at("class-postfix");
      const BasicJSON *pNumber = config->at("Number");
      const BasicJSON *pArray = config->at("Array");
      const BasicJSON *pString = config->at("String");

      if(pString && pString->isCompat(JS_Str)) {
        String = pString->GetString();
        TypeMetaInfo meta;
        if(meta.getTypeInfo(String) == 0) {
          String = "string";
        }
      } else {
        String = "string";
      }

      if(pNumber && pNumber->isCompat(JS_Str)) {
        Number = pNumber->GetString();
        TypeMetaInfo meta;
        if(meta.getTypeInfo(Number) == 0) {
          Number = "auto";
        }
      } else {
        Number = "auto";
      }

      if(pArray && pArray->isCompat(JS_Str)) {
        Array = pArray->GetString();
        TypeMetaInfo meta;
        if(meta.getTypeInfo(Array) == 0) {
          Array = "vector";
        }
      } else {
        Array = "vector";
      }

      if(nameFormat && nameFormat->isCompat(JS_Str)) {
        std::string isCamelCase = nameFormat->GetString();
        LowerCase(&isCamelCase);
        mUseCamelCase = (isCamelCase == "camelcase");
      } else {
        mUseCamelCase = false;
      }

      if(prefix && prefix->isCompat(JS_Str)) {
        mPrefix = prefix->GetString();
      } else {
        mPrefix.clear();
      }
      if(postfix && postfix->isCompat(JS_Str)) {
        mPostfix = postfix->GetString();
      } else {
        mPostfix.clear();
      }

      if(classPrefix && classPrefix->isCompat(JS_Str)) {
        mClassPrefix = classPrefix->GetString();
      } else {
        mClassPrefix.clear();
      }
      if(classPostfix && classPostfix->isCompat(JS_Str)) {
        mClassPostfix = classPostfix->GetString();
      } else {
        mClassPostfix.clear();
      }

    }
  }

  Config_Helper::Config_Helper(const BasicJSON &json, const Config_Helper &parent) : Config_Helper(json) {
    if(!json.HasMember("--config")) {
      String = parent.String;
      Number = parent.Number;
      Array = parent.Array;

      mUseCamelCase = parent.mUseCamelCase;
      mPrefix = parent.mPrefix;
      mPostfix = parent.mPostfix;

      mClassPrefix = parent.mClassPrefix;
      mClassPostfix = parent.mClassPostfix;
    }
  }

  Config_Helper::~Config_Helper() {}

  std::string Config_Helper::getName(const std::string &Name) const {
    std::string out;
    out = mPrefix;
    out += CamelCaseProc(Name);
    out += mPostfix;
    return std::move(out);
  }

  std::string Config_Helper::getClassName(const std::string &className) const {
    std::string out;
    out = mClassPrefix;
    out += CamelCaseProc(className);
    out += mClassPostfix;
    return std::move(out);
  }

  std::string Config_Helper::CamelCaseProc(const std::string &className) const {
    std::string out = className;
    FormatFile(&out);
    if(mUseCamelCase) {
      CamelCaseFormat(&out);
      return out;
    }
    return className;
  }



  bool Json_Helper::setType(const BasicJSON &newType, TypeMetaInfo *info, Config_Helper *config) {
    OriginalJsonName = newType.GetKeyName();
    if(OriginalJsonName.size() == 0) {
      OriginalJsonName = newType.GetString();
    }
    if(OriginalJsonName.find("--") == 0) {
      OriginalJsonName.erase(OriginalJsonName.begin(), OriginalJsonName.begin() + 2);
    }
    std::string str;
    bool autoAdd = false;
    switch(newType.GetType()) {
      case JS_Bool: str = "bool"; break;
      case JS_Str: str = newType.GetString(); break;
      case JS_Int:
      {
        str = config->Number;
        if(str == "auto") {
          if(newType.IsDouble()) {
            str = "double";
          } else if(newType.IsInt64()) {
            str = "int64";
          } else {
            str = "uint64";
          }
        }
        break;
      }
      case JS_Array:
      {
        str = config->Array;
        if(newType.size() != 0) {
          const BasicJSON *child = newType.at(0);
          if(child) {
            if(child->GetType() == JS_Object) {
              PostType = OriginalJsonName;
              CamelCaseFormat(&PostType);
              info->getTypeInfo(PostType, true);
            } else {
              Json_Helper child_help;
              child_help.setType(*child, info, config);
              PostType = child_help.GetType(info, config, true);
            }
          } else {
            PostType = "void";
          }
        } else {
          PostType = "void";
        }
        break;
      }
      case JS_Object:
      {
        str = OriginalJsonName;
        CamelCaseFormat(&str);
        if(str.size() == 0) {
          str = "void";
          TypeName = info->getTypeInfo(str);
        } else {
          TypeName = info->getTypeInfo(str, true);

        }
        break;
      }
      default: str = "void"; break;
    }
    TypeName = info->getTypeInfo(str);
    if(TypeName == 0 && newType.GetType() == JS_Str) {
      TypeName = info->getTypeInfo(config->String);
    }
    return TypeName >= info->LastInternal;
  }

  std::string Json_Helper::GetType(TypeMetaInfo *info, const Config_Helper *config, bool getSimple) const {
    if(getSimple) {
      if(PostType.size() != 0) {
        return PostType;
      }
      return info->AllTypes.at(TypeName).ToString();
    }
    std::string typeString = info->AllTypes.at(TypeName).ToString();

    if(TypeName >= info->LastInternal) {
      typeString = config->getClassName(typeString);
    }
    if(PostType.size() != 0) {
      typeString += '<';
      if(info->getTypeInfo(PostType) >= info->LastInternal) {
        typeString += config->getClassName(PostType);
      } else {
        typeString += PostType;
      }
      typeString += '>';
    }
    return std::move(typeString);
  }

  std::string Json_Helper::ToString(TypeMetaInfo *info, const Config_Helper *config) const {
    std::string out;
    std::string typeString = GetType(info, config);
    if(visible != Public_Visivility) {
      out += NearTox::ToString(visible);
      out += ":\r\n";
    }
    out += typeString;
    out += ' ';
    out += config->getName(ValueName);
    out += ";\r\n";
    bool hasNewVis = visible != Public_Visivility && (UseSeter || UseGeter);
    if(hasNewVis) {
      out += "public:\r\n";
    }
    if(UseSeter) {
      std::string ValueName2 = ValueName;
      CamelCaseFormat(&ValueName2);
      out += fmt::format("void Set{0}(const {1} &new{0}) {{ {2} = {0}new; }}\r\n", ValueName2, typeString, config->getName(ValueName));
    }
    if(UseGeter) {
      std::string ValueName2 = ValueName;
      CamelCaseFormat(&ValueName2);
      out += fmt::format("const {1}& Get{0}() const {{ return {2}; }}\r\n", ValueName2, typeString, config->getName(ValueName));
    }
    if(visible != Public_Visivility) {
      out += "public:\r\n";
    }
    return std::move(out);
  }

  std::string TypeMetaInfo::Type_Info::ToString() const {
    std::string out;
    if(NameSpace.size()) {
      out += NameSpace;
      out += "::";
    }
    out += Name;
    return std::move(out);
  }


  bool TypeMetaInfo::bool_Helper::Parse(const BasicJSON &json, const TypeMetaInfo &info, const Json_Helper &child) {
    bool verdad = true;
    {
      const BasicJSON *childJson = json.at(child.OriginalJsonName);
      if(childJson && childJson->isCompat(JS_Bool)) {
        bool temp = childJson->GetBool();
        verdad &= true;
      } else {
        verdad = false;
      }
    }
    return verdad;
  }

  inline std::string TypeMetaInfo::bool_Helper::makeParser(TypeMetaInfo &info, const Type_Info &typeinfo, const Json_Helper &child, Config_Helper &config, const std::string &variableName) {
    std::string out = "{\r\n";
    if(variableName == "childJson") {
      out += fmt::format("const NearTox::BasicJSON *{} = json.at(\"{}\");\r\n", variableName, child.OriginalJsonName);
    }
    out += fmt::format("if({0} && {0}->isCompat(NearTox::JS_Bool)) {{\r\n", variableName);
    out += config.getName(child.ValueName);
    out += fmt::format(" = {0}->GetBool();\r\n", variableName);
    out += fmt::format("}} else {{\r\nassert({} != nullptr);\r\nverdad = false;\r\n}}\r\n"
      "}}\r\n", variableName);
    return std::move(out);
  }

  bool TypeMetaInfo::str_Helper::Parse(const BasicJSON &json, const TypeMetaInfo &info, const Json_Helper &child) {
    const auto &typeinfo = info.AllTypes.at(child.TypeName);
    bool verdad = true;
    {
      const BasicJSON *childJson = json.at(child.OriginalJsonName);
      if(childJson && childJson->isCompat(JS_Str)) {
        if(typeinfo.Name == "string") {
          std::string temp = childJson->GetString();
          verdad &= true;
        } else if(typeinfo.Name == "wstring") {
          std::wstring temp = LToString(childJson->GetString());
          verdad &= true;
        } else if(typeinfo.Name == "PathDir") {
          PathDir temp = PathDir::Make(childJson->GetString());
          verdad &= temp.isValid();
        } else if(typeinfo.Name == "URLAnalizer") {
          URLAnalizer temp = URLAnalizer(childJson->GetString());
          verdad &= (temp.GetProtocol() != URL_ERROR);
        }
      } else {
        verdad &= false;
      }
    }
    return verdad;
  }

  std::string TypeMetaInfo::str_Helper::makeParser(TypeMetaInfo &info, const Type_Info &typeinfo, const Json_Helper &child, Config_Helper &config, const std::string &variableName) {
    std::string out = "{\r\n";
    if(variableName == "childJson") {
      out += fmt::format("const NearTox::BasicJSON *{} = json.at(\"{}\");\r\n", variableName, child.OriginalJsonName);
    }
    out += fmt::format("if({0} && {0}->isCompat(NearTox::JS_Str)) {{\r\n", variableName);
    out += config.getName(child.ValueName);
    out += " = ";
    if(typeinfo.Name == "string") {
      out += fmt::format("{0}->GetString();\r\n", variableName);
    } else if(typeinfo.Name == "wstring") {
      out += fmt::format("NearTox::LToString({0}->GetString());\r\n", variableName);
    } else if(typeinfo.Name == "PathDir") {
      out += fmt::format("NearTox::PathDir::Make({0}->GetString());\r\n", variableName);
      out += fmt::format("verdad &= {}.isValid();\r\n", config.getName(child.ValueName));
    } else if(typeinfo.Name == "URLAnalizer") {
      out += fmt::format("NearTox::URLAnalizer({0}->GetString());\r\n", variableName);
      out += fmt::format("verdad &= ({}.GetProtocol() != NearTox::URL_ERROR);\r\n", config.getName(child.ValueName));
    }
    out += fmt::format("}} else {{\r\nassert({} != nullptr);\r\nverdad = false;\r\n}}\r\n"
      "}}\r\n", variableName);
    return std::move(out);
  }

  bool TypeMetaInfo::int_Helper::Parse(const BasicJSON &json, const TypeMetaInfo &info, const Json_Helper &child) {
    const auto &typeinfo = info.AllTypes.at(child.TypeName);
    bool verdad = true;
    {
      const BasicJSON *childJson = json.at(child.OriginalJsonName);
      if(childJson && childJson->isCompat(JS_Int)) {
        if(typeinfo.Name == "double" || typeinfo.Name == "float") {
          double temp = static_cast<float>(childJson->GetInt64());
        }
        verdad &= true;
      } else {
        verdad &= false;
      }
    }
    return verdad;
  }

  std::string TypeMetaInfo::int_Helper::makeParser(TypeMetaInfo &info, const Type_Info &typeinfo, const Json_Helper &child, Config_Helper &config, const std::string &variableName) {
    std::string out = "{\r\n";
    if(variableName == "childJson") {
      out += fmt::format("const NearTox::BasicJSON *{} = json.at(\"{}\");\r\n", variableName, child.OriginalJsonName);
    }
    out += fmt::format("if({0} && {0}->isCompat(NearTox::JS_Int)) {{\r\n", variableName);
    out += config.getName(child.ValueName);
    out += " = ";
    if(typeinfo.Name == "double") {
      out += fmt::format("{0}->GetDouble();\r\n", variableName);
    } else if(typeinfo.Name == "float") {
      out += fmt::format("static_cast<float>({0}->GetDouble());\r\n", variableName);
    } else if(typeinfo.Name == "int8_t" || typeinfo.Name == "int16_t" || typeinfo.Name == "int32_t" || typeinfo.Name == "int64_t") {
      out += fmt::format("static_cast<{1}>({0}->GetInt64());\r\n", variableName, typeinfo.Name);
    } else {
      out += fmt::format("static_cast<{1}>({0}->GetUInt64());\r\n", variableName, typeinfo.Name);
    }
    out += fmt::format("}} else {{\r\nassert({} != nullptr);\r\nverdad = false;\r\n}}\r\n"
      "}}\r\n", variableName);
    return std::move(out);
  }

  std::string TypeMetaInfo::vector_Helper::makeParser(TypeMetaInfo &info, const Type_Info &typeinfo, const Json_Helper &child, Config_Helper &config, const std::string &variableName) {
    std::string out = "{\r\n";
    if(variableName == "childJson") {
      out += fmt::format("const NearTox::BasicJSON *{} = json.at(\"{}\");\r\n", variableName, child.OriginalJsonName);
    }
    out += fmt::format("if({0} && {0}->isCompat(NearTox::JS_Array)) {{\r\n", variableName);
    out += fmt::format("for(size_t i = 0; i < {}->size(); ++i) {{\r\n", variableName);
    out += fmt::format("const NearTox::BasicJSON *sub_{0}= {0}->at(i);\r\n", variableName);
    Json_Helper newChild;
    if(child.PostType.size() != 0) {
      newChild.setType(BasicJSON::MakeStr(child.PostType), &info, &config);
    } else {
      newChild = child;
    }
    newChild.ValueName = "newValue";
    out += fmt::format("{} {};\r\n", newChild.GetType(&info, &config), config.getName("newValue"));
    auto &helper_info = info.AllTypes.at(newChild.TypeName);
    if(helper_info.helper) {
      out += helper_info.helper->makeParser(info, helper_info, newChild, config, "sub_" + variableName);
    } else {
      out += "{\r\n";
      if(variableName == "sub_childJson") {
        out += fmt::format("const NearTox::BasicJSON *{} = json.at(\"{}\");\r\n", "sub_" + variableName, child.OriginalJsonName);
      }
      out += fmt::format("if({0} && {0}->isCompat(NearTox::JS_Object)) {{\r\n", "sub_" + variableName);
      out += fmt::format("verdad &= {}.Parse(*{});\r\n", config.getName("newValue"), "sub_" + variableName);
      out += fmt::format("}} else {{\r\nassert({} != nullptr);\r\nverdad = false;\r\n}}\r\n"
        "}}\r\n", "sub_" + variableName);
    }
    out += fmt::format("{}.push_back(std::move({}));\r\n", config.getName(child.ValueName), config.getName("newValue"));
    out += fmt::format("}}\r\n}} else {{\r\nassert({} != nullptr);\r\nverdad = false;\r\n}}\r\n"
      "}}\r\n", variableName);
    return std::move(out);
  }

  TypeMetaInfo::TypeMetaInfo() {
    AllTypes.push_back({"void", "", nullptr});
    AllTypes.push_back({"bool", "", std::make_shared<bool_Helper>()});
    AllTypes.push_back({"int8_t", "", std::make_shared<int_Helper>(), "int8"});
    AllTypes.push_back({"int16_t", "", std::make_shared<int_Helper>(), "int16"});
    AllTypes.push_back({"int32_t", "", std::make_shared<int_Helper>(), "int32"});
    AllTypes.push_back({"int64_t", "", std::make_shared<int_Helper>(), "int64"});
    AllTypes.push_back({"uint8_t", "", std::make_shared<int_Helper>(), "uint8"});
    AllTypes.push_back({"uint16_t", "", std::make_shared<int_Helper>(), "uint16"});
    AllTypes.push_back({"uint32_t", "", std::make_shared<int_Helper>(), "uint32"});
    AllTypes.push_back({"uint64_t", "", std::make_shared<int_Helper>(), "uint64"});
    AllTypes.push_back({"size_t", "", std::make_shared<int_Helper>(), "size"});
    AllTypes.push_back({"double", "", std::make_shared<int_Helper>()});
    AllTypes.push_back({"float", "", std::make_shared<int_Helper>()});

    AllTypes.push_back({"string", "std", std::make_shared<str_Helper>()});
    AllTypes.push_back({"wstring", "std", std::make_shared<str_Helper>()});
    AllTypes.push_back({"vector", "std", std::make_shared<vector_Helper>()});
    AllTypes.push_back({"deque", "std", std::make_shared<vector_Helper>()});

    AllTypes.push_back({"URLAnalizer", "NearTox", std::make_shared<str_Helper>(), "url"});
    AllTypes.push_back({"PathDir", "NearTox", std::make_shared<str_Helper>(), "file"});

    LastInternal = AllTypes.size();
  }

  TypeID TypeMetaInfo::getTypeInfo(const std::string &find_type, bool autoAdd) {
    std::string Name = find_type;
    std::string NameSpace;
    {
      size_t pos = Name.rfind("::");
      if(pos != npos) {
        NameSpace = Name.substr(0, pos);
        Name = Name.substr(pos + 2);
      }
    }
    for(size_t i = 0; i < AllTypes.size(); i++) {
      if(icompare(AllTypes.at(i).Alt, Name)) {
        return i;
      } else if(icompare(AllTypes.at(i).Name, Name)) {
        return i;
      }
    }
    if(autoAdd) {
      AllTypes.push_back({Name, NameSpace});
      return AllTypes.size() - 1;
    }
    return 0;
  }

}