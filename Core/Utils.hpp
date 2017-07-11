//
// Copyright 2017: Ernesto Ramirez(NearTox)
// TODO: Put a Licence Here
// Meanwhile... I wrote this code in hope that someone improve this code
//

#ifndef Core_Utils_HPP
#define Core_Utils_HPP

#include <string>
#include <algorithm>

namespace NearTox {

  void FormatFile(std::string *toRemove);

  void CamelCaseFormat(std::string *out);

  enum Visivility : uint8_t {
    Public_Visivility = 0,
    Protected_Visivility = 1,
    Private_Visivility = 2,
  };

  std::string ToString(const Visivility &vis);
}

#endif // Core_Utils_HPP