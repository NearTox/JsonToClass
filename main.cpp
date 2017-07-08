//
// Copyright 2017: Ernesto Ramirez<NearTox@outlook.com>
// TODO: Put a Licence Here
// Meanwhile... I wrote this code in hope that someone improve this code
//

#include "Core/JsonToClass.hpp"

#include <NearTox/Base.hpp>
#include <NearTox/File.hpp>

#include <iostream>

namespace NearTox {

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
    if(filename_arg.size() > 2 && filename_arg.front() == L'\'' && filename_arg.back() == L'\'') {
      return PathDir::Make(filename_arg.substr(1, filename_arg.size() - 2));
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
  //NearTox::worker(NearTox::makePath(L"T:\\NT.Sync\\JsonToClass\\result.json")); 
  std::cout << "JsonToClass.exe [] [--] [<pathspec>...]\n\n"
    "\t<pathspec>...\t\tArchivos a ser procesados\n\n";
  system("pause");
  return 0;
}
