#include <NearTox/Base.hpp>
#include <NearTox/Json.hpp>
#include <NearTox/File.hpp>

#include <NearTox/Internet/UrlAnalizer.hpp>
#include <fmt/format.h>

#include <iostream>
#include <map>

namespace NearTox {

  static std::string Dic("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_");
  static void FormatFile(std::string *toRemove) {
    std::replace_if(toRemove->begin(), toRemove->end(), [](const char &c) {
      return Dic.find(c) == npos;
    }, '_');
    auto last = std::unique(toRemove->begin(), toRemove->end(), [](const char &l, const char &r) {
      return l == '_' && l == r;
    });

    toRemove->erase(last, toRemove->end());
    if(toRemove->front() == L'_') {
      *toRemove = toRemove->substr(1);
    }
    if(toRemove->back() == '_') {
      *toRemove = toRemove->substr(0, toRemove->length() - 1);
    }
  }

  static void CamelCaseFormat(std::string *out) {
    bool upercase = true;
    for(char &DataPos : *out) {
      if(upercase) {
        DataPos = static_cast<char>(toupper(DataPos));
        upercase = false;
      }
      if(!(DataPos >= 'a' && DataPos <= 'z') && !(DataPos >= 'A' && DataPos <= 'Z') && !isInt(DataPos)) {
        upercase = true;
      }
    }
  }

  enum Visivility : uint8_t {
    Public_Visivility = 0,
    Protected_Visivility = 1,
    Private_Visivility = 2,
  };

  std::string ToString(Visivility vis) {
    switch(vis) {
      case Public_Visivility: return "public";
      case Protected_Visivility: return "protected";
      case Private_Visivility: return "private";
      default: return "";
    }
  }
  typedef size_t TypeID;


  class Config_Helper {
  public:
    std::string String = "string";
    std::string Number = "auto";
    std::string Array = "vector";

    Config_Helper(const BasicJSON &json);
    Config_Helper(const BasicJSON &json, const Config_Helper &parent) : Config_Helper(json) {
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
    ~Config_Helper() {}

    std::string getName(const std::string &Name) const {
      std::string out;
      out = mPrefix;
      out += CamelCaseProc(Name);
      out += mPostfix;
      return std::move(out);
    }

    std::string getClassName(const std::string &className) const {
      std::string out;
      out = mClassPrefix;
      out += CamelCaseProc(className);
      out += mClassPostfix;
      return std::move(out);
    }

  private:
    bool mUseCamelCase = false;
    std::string mPrefix;
    std::string mPostfix;

    std::string mClassPrefix;
    std::string mClassPostfix;

    std::string CamelCaseProc(const std::string &className) const {
      std::string out = className;
      FormatFile(&out);
      if(mUseCamelCase) {
        CamelCaseFormat(&out);
        return out;
      }
      return className;
    }
    //CLASS_NO_COPY(Config_Helper);
  };

  struct TypeMetaInfo;
  struct Json_Helper {

    std::string OriginalJsonName;
    std::string ValueName;
    std::string PostType; // type<xx> 
    TypeID TypeName = 0; // void
    Visivility visible = Public_Visivility;
    bool UseSeter = false;
    bool UseGeter = false;

    bool setType(const BasicJSON &newType, TypeMetaInfo *info, Config_Helper *config);
    std::string GetType(TypeMetaInfo *info, const Config_Helper *config, bool getSimple = false) const;
    std::string ToString(TypeMetaInfo *info, const Config_Helper *config) const;
  };

  struct TypeMetaInfo {
    TypeID LastInternal;
    struct Type_Info;
    struct Parser_Helper {
      virtual std::string makeParser(TypeMetaInfo &info, const Type_Info &typeinfo, const Json_Helper &child, Config_Helper &config, const std::string &variableName = "childJson") = 0;
    };
    struct bool_Helper : public Parser_Helper {
      bool Parse(const BasicJSON &json, const TypeMetaInfo &info, const Json_Helper &child) {
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
      std::string makeParser(TypeMetaInfo &info, const Type_Info &typeinfo, const Json_Helper &child, Config_Helper &config, const std::string &variableName) {
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
    };
    struct int_Helper : public Parser_Helper {
      bool Parse(const BasicJSON &json, const TypeMetaInfo &info, const Json_Helper &child) {
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
      std::string makeParser(TypeMetaInfo &info, const Type_Info &typeinfo, const Json_Helper &child, Config_Helper &config, const std::string &variableName) {
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
    };
    struct str_Helper : public Parser_Helper {
      bool Parse(const BasicJSON &json, const TypeMetaInfo &info, const Json_Helper &child) {
        const auto &typeinfo = info.AllTypes.at(child.TypeName);
        bool verdad = true;
        {
          const BasicJSON *childJson = json.at(child.OriginalJsonName);
          if(childJson && childJson->isCompat(JS_Str)) {
            if(typeinfo.Name == "string") {
              std::string temp = childJson->GetString();
              verdad &= true;
            } else if(typeinfo.Name == "wstring") {
              std::wstring temp = NearTox::LToString(childJson->GetString());
              verdad &= true;
            } else if(typeinfo.Name == "PathDir") {
              NearTox::PathDir temp = NearTox::PathDir::Make(childJson->GetString());
              verdad &= temp.isValid();
            } else if(typeinfo.Name == "URLAnalizer") {
              NearTox::URLAnalizer temp = NearTox::URLAnalizer(childJson->GetString());
              verdad &= (temp.GetProtocol() != URL_ERROR);
            }
          } else {
            verdad &= false;
          }
        }
        return verdad;
      }
      std::string makeParser(TypeMetaInfo &info, const Type_Info &typeinfo, const Json_Helper &child, Config_Helper &config, const std::string &variableName) {
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
    };
    struct vector_Helper : public Parser_Helper {
      std::string makeParser(TypeMetaInfo &info, const Type_Info &typeinfo, const Json_Helper &child, Config_Helper &config, const std::string &variableName) {
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
    };
    struct Type_Info {
      std::string Name = "";
      std::string NameSpace = "";
      std::shared_ptr<Parser_Helper> helper;
      std::string Alt = "";
      std::string ToString() const {
        std::string out;
        if(NameSpace.size()) {
          out += NameSpace;
          out += "::";
        }
        out += Name;
        return std::move(out);
      }
    };

    std::vector<Type_Info> AllTypes;

    TypeMetaInfo() {
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
    ~TypeMetaInfo() {}

    TypeID getTypeInfo(const std::string &find_type, bool autoAdd = false) {
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

  };

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

  class JsonToClass {
  private:
    typedef std::vector<JsonToClass> Child_info;
    std::shared_ptr<Child_info> Childs;
    TypeMetaInfo meta;
  public:
    std::string name;
    Config_Helper Config;
    std::vector<Json_Helper> memberList;
    bool isValid(const std::string &str) {
      return !(str == "--config" ||
        str == "public" ||
        str == "private" ||
        str == "protected" ||
        str == "--geter" ||
        str == "--seter");
    }

    JsonToClass(BasicJSON &json, const Config_Helper &parent) :
      Config(json, parent) {

      BasicJSON *publicMembers = json.at("public", JS_Array);
      BasicJSON *privateMembers = json.at("private", JS_Array);
      BasicJSON *protectedMembers = json.at("protected", JS_Array);
      BasicJSON *geterMembers = json.at("--geter", JS_Array);
      BasicJSON *seterMembers = json.at("--seter", JS_Array);

      for(BasicJSON *for_itt : json) {
        if(for_itt && isValid(for_itt->GetKeyName())) {
          Json_Helper newMember;
          newMember.ValueName = for_itt->GetKeyName();
          newMember.setType(*for_itt, &meta, &Config);
          if(for_itt->GetType() == JS_Array && for_itt->size() != 0) {
            // unknowtype
            BasicJSON *child = for_itt->at(0, JS_Null);
            if(child && child->GetType() == JS_Object) {
              for_itt = child;
            }
          }
          if(for_itt->GetType() == JS_Object) {
            if(Childs == nullptr) {
              Childs = std::make_shared<Child_info>();
            }
            Childs->push_back(JsonToClass(*for_itt, Config));
            Childs->back().name = newMember.GetType(&meta, &Config, true);
          }
          if(newMember.ValueName.find("--") != 0) {
            memberList.push_back(newMember);
          }
        }
      }

      if(geterMembers) {
        if(geterMembers->isCompat(JS_Str)) {
          bool allGeter = icompare(geterMembers->GetString(), "all");
          for(auto &for_itt : memberList) {
            for_itt.UseGeter = allGeter;
          }
        } else if(geterMembers->isCompat(JS_Array)) {
          for(BasicJSON *members : *geterMembers) {
            if(members && members->isCompat(JS_Str)) {
              std::string str = members->GetString();
              for(auto &for_itt : memberList) {
                if(icompare(str, for_itt.ValueName)) {
                  for_itt.UseGeter = true;
                  break;
                }
              }
            }
          }
        }
      }

      if(seterMembers) {
        if(seterMembers->isCompat(JS_Str)) {
          bool allSeter = icompare(seterMembers->GetString(), "all");
          for(auto &for_itt : memberList) {
            for_itt.UseSeter = allSeter;
          }
        } else if(seterMembers->isCompat(JS_Array)) {
          for(BasicJSON *members : *seterMembers) {
            if(members && members->isCompat(JS_Str)) {
              std::string str = members->GetString();
              for(auto &for_itt : memberList) {
                if(icompare(str, for_itt.ValueName)) {
                  for_itt.UseSeter = true;
                  break;
                }
              }
            }
          }
        }
      }

      if(privateMembers) {
        if(privateMembers->isCompat(JS_Str)) {
          bool allPrivate = icompare(privateMembers->GetString(), "all");
          if(allPrivate) {
            for(auto &for_itt : memberList) {
              for_itt.visible = Private_Visivility;
            }
          }
        } else if(privateMembers->isCompat(JS_Array)) {
          for(BasicJSON *members : *privateMembers) {
            if(members && members->isCompat(JS_Str)) {
              std::string str = members->GetString();
              for(auto &for_itt : memberList) {
                if(icompare(str, for_itt.ValueName)) {
                  for_itt.visible = Private_Visivility;
                  break;
                }
              }
            }
          }
        }
      }
      if(protectedMembers) {
        if(protectedMembers->isCompat(JS_Str)) {
          bool allProtected = icompare(protectedMembers->GetString(), "all");
          if(allProtected) {
            for(auto &for_itt : memberList) {
              if(for_itt.visible < Protected_Visivility) {
                for_itt.visible = Protected_Visivility;
              }
            }
          }
        } else if(protectedMembers->isCompat(JS_Array)) {
          for(BasicJSON *members : *protectedMembers) {
            if(members && members->isCompat(JS_Str)) {
              std::string str = members->GetString();
              for(auto &for_itt : memberList) {
                if(icompare(str, for_itt.ValueName)) {
                  for_itt.visible = Protected_Visivility;
                  break;
                }
              }
            }
          }
        }
      }
      if(publicMembers) {
        if(publicMembers->isCompat(JS_Str)) {
          bool allPublic = icompare(publicMembers->GetString(), "all");
          if(allPublic) {
            for(auto &for_itt : memberList) {
              if(for_itt.visible < Public_Visivility) {
                for_itt.visible = Public_Visivility;
              }
            }
          }
        } else if(publicMembers->isCompat(JS_Array)) {
          for(BasicJSON *members : *publicMembers) {
            if(members && members->isCompat(JS_Str)) {
              std::string str = members->GetString();
              for(auto &for_itt : memberList) {
                if(icompare(str, for_itt.ValueName)) {
                  for_itt.visible = Public_Visivility;
                  break;
                }
              }
            }
          }
        }
      }
    }
    ~JsonToClass() {}

    std::string save() {
      std::string out;
      out = fmt::format("struct {} {{\r\n", Config.getClassName(name));
      if(Childs && Childs->size() != 0) {
        for(auto &for_itt : *Childs) {
          out += for_itt.save();
        }
      }
      for(const auto &for_itt : memberList) {
        out += for_itt.ToString(&meta, &Config);
      }
      out += makeParse();
      out += "};\r\n";
      return std::move(out);
    }

    std::string makeParse() {
      std::string out = "bool Parse(const NearTox::BasicJSON &json) {\r\n"
        "bool verdad = true;\r\n";

      for(const auto &for_itt : memberList) {
        auto helper = meta.AllTypes.at(for_itt.TypeName).helper;
        if(helper) {
          out += helper->makeParser(meta, meta.AllTypes.at(for_itt.TypeName), for_itt, Config);
        } else {
          out += "{\r\n";
          out += fmt::format("const NearTox::BasicJSON *{} = json.at(\"{}\");\r\n", "childJson", for_itt.OriginalJsonName);
          out += fmt::format("if({0} && {0}->isCompat(NearTox::JS_Object)) {{\r\n", "childJson");
          out += fmt::format("verdad &= {}.Parse(*{});\r\n", Config.getName(for_itt.ValueName), "childJson");
          out += fmt::format("}} else {{\r\nassert({} != nullptr);\r\nverdad = false;\r\n}}\r\n"
            "}}\r\n", "childJson");
        }
      }
      out += "return verdad;\r\n}\r\n";
      return std::move(out);
    }

  };

  void worker(const PathDir &dir) {
    BasicJSON MainJson;
    {
      std::string temp;
      Read(dir, &temp);
      MainJson.Parse(temp);
    }
    PathDir copy = dir;
    Config_Helper MainConfig(MainJson);
    for(BasicJSON *for_itt : MainJson) {
      if(for_itt && for_itt->GetKeyName() != "--config") {
        JsonToClass process(*for_itt, MainConfig);
        process.name = for_itt->GetKeyName();
        std::string out = process.save();
        copy.SetFileName(LToString(process.Config.getClassName(process.name) + ".hpp"));
        Write(copy, out.c_str(), out.size());
      }
    }
  }

  PathDir makePath(const std::wstring &filename_arg) {
    if(filename_arg.size() > 2) {
      wchar_t start = filename_arg[0];
      wchar_t end = *(filename_arg.end() - 1);
      if(start == L'\'' && end == L'\'') {
        return PathDir::Make(filename_arg.substr(1, filename_arg.size() - 2));
      }
    }
    return PathDir::Make(filename_arg);
  }
}

int main() {
  SetConsoleTitle(L"Escrito por NearTox");
  int argc = 0;
  wchar_t **wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
  if(!wargv) {
    return 0;
  }
  for(int i = 1; i < argc; i++) {
    NearTox::worker(NearTox::makePath(NearTox::LToString(wargv[i])));
  }
  LocalFree(wargv);
  if(argc > 1) {
    return 0;
  }
  NearTox::worker(NearTox::makePath(L"T:\\NT.Sync\\JsonToClass\\result.json"));
  std::cout << "JsonToClass.exe [] [--] [<pathspec>...]\n\n"
    "\t<pathspec>...\t\tArchivos a ser procesados\n\n";
  system("pause");
  return 0;
}
