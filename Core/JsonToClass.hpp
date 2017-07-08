//
// Copyright 2017: Ernesto Ramirez<NearTox@outlook.com>
// TODO: Put a Licence Here
// Meanwhile... I wrote this code in hope that someone improve this code
//

#ifndef Core_JsonToClass_HPP
#define Core_JsonToClass_HPP

#include "TypeMetaInfo.hpp"

namespace NearTox {

  class JsonToClass {
  private:
    typedef std::vector<JsonToClass> Child_info;
    std::shared_ptr<Child_info> Childs;
  public:
    std::string name;
    Config_Helper Config;
    std::vector<Json_Helper> memberList;

    bool isValid(const std::string &str);

    JsonToClass(BasicJSON &json, const Config_Helper &parent);
    ~JsonToClass();

    std::string save();

    std::string makeParse();

  };

}

#endif // Core_JsonToClass_HPP