//
// Copyright 2017: Ernesto Ramirez(NearTox)
// TODO: Put a Licence Here
// Meanwhile... I wrote this code in hope that someone improve this code
//

#ifndef Core_TypeMetaInfo_HPP
#define Core_TypeMetaInfo_HPP

#include "Utils.hpp"
#include <NearTox/Json.hpp>
#include <NearTox/UniqueClass.hpp>
#include <memory>

namespace NearTox {

  class Config_Helper {
  public:
    std::string String = "string";
    std::string Number = "auto";
    std::string Array = "vector";

    Config_Helper(const BasicJSON &json);
    Config_Helper(const BasicJSON &json, const Config_Helper &parent);
    ~Config_Helper();

    std::string getName(const std::string &Name) const;

    std::string getClassName(const std::string &className) const;

  private:
    bool mUseCamelCase = false;
    std::string mPrefix;
    std::string mPostfix;

    std::string mClassPrefix;
    std::string mClassPostfix;

    std::string CamelCaseProc(const std::string &className) const;
    //CLASS_NO_COPY(Config_Helper);
  };


  typedef size_t TypeID;

  struct Json_Helper {

    std::string OriginalJsonName;
    std::string ValueName;
    std::string PostType; // type<xx>
    TypeID TypeName = 0; // void
    Visivility visible = Public_Visivility;
    bool UseSeter = false;
    bool UseGeter = false;
    bool IsOptional = false;

    bool setType(const BasicJSON &newType, Config_Helper *config);
    std::string GetType(const Config_Helper *config, bool getSimple = false) const;
    std::string ToString(const Config_Helper *config) const;
  };


  struct TypeMetaInfo: public UniqueHandle {

    struct Type_Info;
    struct Parser_Helper {
      virtual std::string makeParser(const Type_Info &typeinfo, const Json_Helper &child,
        Config_Helper &config, const std::string &variableName = "childJson") = 0;
    };

    struct Type_Info {
      std::string Name;
      std::string NameSpace;
      std::shared_ptr<Parser_Helper> helper;
      std::string Alt;

      Type_Info(std::string &&name, std::string &&nameSpace,
        std::shared_ptr<Parser_Helper> &&helper = std::shared_ptr<Parser_Helper>(),
        const std::string &&alt = std::string()
      );

      std::string ToString() const;
    };

    struct bool_Helper : public Parser_Helper {
      std::string makeParser(const Type_Info &typeinfo, const Json_Helper &child, Config_Helper &config, const std::string &variableName);
    };

    struct int_Helper : public Parser_Helper {
      std::string makeParser(const Type_Info &typeinfo, const Json_Helper &child, Config_Helper &config, const std::string &variableName);
    };

    struct str_Helper : public Parser_Helper {
      std::string makeParser(const Type_Info &typeinfo, const Json_Helper &child, Config_Helper &config, const std::string &variableName);
    };

    struct vector_Helper : public Parser_Helper {
      std::string makeParser(const Type_Info &typeinfo, const Json_Helper &child, Config_Helper &config, const std::string &variableName);
    };

    TypeID LastInternal;

    std::vector<Type_Info> AllTypes;

    TypeMetaInfo();
    ~TypeMetaInfo();

    TypeID getTypeInfo(const std::string &find_type, bool autoAdd = false);

    CLASS_NO_COPY(TypeMetaInfo);
  };

}

#endif // Core_TypeMetaInfo_HPP