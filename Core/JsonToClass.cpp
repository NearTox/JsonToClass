//
// Copyright 2017: Ernesto Ramirez<NearTox@outlook.com>
// TODO: Put a Licence Here
// Meanwhile... I wrote this code in hope that someone improve this code
//

#include "JsonToClass.hpp"

#include <NearTox/Base.hpp>

#include <fmt/format.h>

namespace NearTox {
  
  bool JsonToClass::isValid(const std::string & str) {
    return !(str == "--config" ||
      str == "public" ||
      str == "private" ||
      str == "protected" ||
      str == "--geter" ||
      str == "--seter");
  }
  
  JsonToClass::JsonToClass(BasicJSON &json, const Config_Helper &parent) :
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

  JsonToClass::~JsonToClass() {}

  std::string JsonToClass::save() {
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

  std::string JsonToClass::makeParse() {
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

}