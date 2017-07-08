//
// Copyright 2017: Ernesto Ramirez<NearTox@outlook.com>
// TODO: Put a Licence Here
// Meanwhile... I wrote this code in hope that someone improve this code
//

#include "Utils.hpp"
#include <NearTox/Base.hpp>

#include <stdexcept>

namespace NearTox {

  static std::string Dic("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_");
  void FormatFile(std::string *toRemove) {
    if(toRemove == nullptr) {
      throw std::invalid_argument("toRemove is null");
    }
    std::replace_if(toRemove->begin(), toRemove->end(), [](const char &c) {
      return Dic.find(c) == std::string::npos;
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

  void CamelCaseFormat(std::string *out) {
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

  std::string ToString(const Visivility &vis) {
    switch(vis) {
      case Public_Visivility: return "public";
      case Protected_Visivility: return "protected";
      case Private_Visivility: return "private";
      default: return "";
    }
  }

}