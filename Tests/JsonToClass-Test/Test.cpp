//
// Copyright 2017: Ernesto Ramirez<NearTox@outlook.com>
// TODO: Put a Licence Here
// Meanwhile... I wrote this code in hope that someone improve this code
//

#include "gtest/gtest.h"

#include "../../Core/Utils.cpp"
#include "../../Core/TypeMetaInfo.cpp"

namespace NearTox {

  std::string FormatFile_Test(const std::string &out) {
    std::string temp = out;
    FormatFile(&temp);
    return temp;
  }
  std::string CamelCaseFormat_Test(const std::string &out) {
    std::string temp = out;
    CamelCaseFormat(&temp);
    return temp;
  }
}

using namespace NearTox;

TEST(JsonToClass, Utils) {
  EXPECT_EQ("Test", CamelCaseFormat_Test("test"));
  EXPECT_EQ("TesT", CamelCaseFormat_Test("TesT"));
  EXPECT_EQ("TEST1", CamelCaseFormat_Test("TEST1"));
  EXPECT_EQ("TesT-Test", CamelCaseFormat_Test("TesT-test"));
  EXPECT_EQ("Test1_Test", CamelCaseFormat_Test("Test1_test"));

  EXPECT_EQ("test", FormatFile_Test("test"));
  EXPECT_EQ("test_1", FormatFile_Test("test_1"));
  // TODO: invalid name on compiler [\d.f]+_.*
  EXPECT_EQ("1_test", FormatFile_Test("1 test"));

  EXPECT_EQ("test_goal", FormatFile_Test("@#+test++_\\&|/^*-:+goal++"));

}