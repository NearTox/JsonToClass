struct m_Test_native {
  struct m_Param_native {
    bool Yeah;
    std::vector<m_Param_native> Array;
    std::vector<std::string> Array2;
    bool Parse(const NearTox::BasicJSON &json) {
      bool verdad = true;
      {
        const NearTox::BasicJSON *childJson = json.at("yeah");
        if(childJson && childJson->isCompat(NearTox::JS_Bool)) {
          Yeah = childJson->GetBool();
        } else {
          assert(childJson != nullptr);
          verdad = false;
        }
      }
      {
        const NearTox::BasicJSON *childJson = json.at("array");
        if(childJson && childJson->isCompat(NearTox::JS_Array)) {
          for(size_t i = 0; i < childJson->size(); ++i) {
            const NearTox::BasicJSON *sub_childJson = childJson->at(i);
            m_Param_native NewValue;
            {
              if(sub_childJson && sub_childJson->isCompat(NearTox::JS_Object)) {
                verdad &= NewValue.Parse(*sub_childJson);
              } else {
                assert(sub_childJson != nullptr);
                verdad = false;
              }
            }
            Array.push_back(std::move(NewValue));
          }
        } else {
          assert(childJson != nullptr);
          verdad = false;
        }
      }
      {
        const NearTox::BasicJSON *childJson = json.at("array2");
        if(childJson && childJson->isCompat(NearTox::JS_Array)) {
          for(size_t i = 0; i < childJson->size(); ++i) {
            const NearTox::BasicJSON *sub_childJson = childJson->at(i);
            std::string NewValue;
            {
              if(sub_childJson && sub_childJson->isCompat(NearTox::JS_Str)) {
                NewValue = sub_childJson->GetString();
              } else {
                assert(sub_childJson != nullptr);
                verdad = false;
              }
            }
            Array2.push_back(std::move(NewValue));
          }
        } else {
          assert(childJson != nullptr);
          verdad = false;
        }
      }
      return verdad;
    }
  };
  m_Param_native Param;
  std::shared_ptr<m_Param_native> Param_Opt;
  uint64_t Args;
  NearTox::PathDir File;
  NearTox::PathDir File_Opt;
  NearTox::URLAnalizer Url;
  double Time;
  std::string String;
  std::wstring Wstring;
  std::vector<void> MyArray;
  std::vector<std::wstring> MyWArray;
  bool Parse(const NearTox::BasicJSON &json) {
    bool verdad = true;
    {
      const NearTox::BasicJSON *childJson = json.at("param");
      if(childJson && childJson->isCompat(NearTox::JS_Object)) {
        verdad &= Param.Parse(*childJson);
      } else {
        assert(childJson != nullptr);
        verdad = false;
      }
    }
    {
      const NearTox::BasicJSON *childJson = json.at("param_opt");
      if(childJson && childJson->isCompat(NearTox::JS_Object)) {
        Param_Opt = std::make_shared<m_Param_native>();
        verdad &= Param_Opt->Parse(*childJson);
      }
    }
    {
      const NearTox::BasicJSON *childJson = json.at("args");
      if(childJson && childJson->isCompat(NearTox::JS_Int)) {
        Args = static_cast<uint64_t>(childJson->GetUInt64());
      } else {
        assert(childJson != nullptr);
        verdad = false;
      }
    }
    {
      const NearTox::BasicJSON *childJson = json.at("file");
      if(childJson && childJson->isCompat(NearTox::JS_Str)) {
        File = NearTox::PathDir::Make(childJson->GetString());
        verdad &= File.isValid();
      } else {
        assert(childJson != nullptr);
        verdad = false;
      }
    }
    {
      const NearTox::BasicJSON *childJson = json.at("file_opt");
      if(childJson && childJson->isCompat(NearTox::JS_Str)) {
        File_Opt = NearTox::PathDir::Make(childJson->GetString());
        verdad &= File_Opt.isValid();
      }
    }
    {
      const NearTox::BasicJSON *childJson = json.at("url");
      if(childJson && childJson->isCompat(NearTox::JS_Str)) {
        Url = NearTox::URLAnalizer(childJson->GetString());
        verdad &= (Url.GetProtocol() != NearTox::URL_ERROR);
      } else {
        assert(childJson != nullptr);
        verdad = false;
      }
    }
    {
      const NearTox::BasicJSON *childJson = json.at("time");
      if(childJson && childJson->isCompat(NearTox::JS_Int)) {
        Time = childJson->GetDouble();
      } else {
        assert(childJson != nullptr);
        verdad = false;
      }
    }
    {
      const NearTox::BasicJSON *childJson = json.at("string");
      if(childJson && childJson->isCompat(NearTox::JS_Str)) {
        String = childJson->GetString();
      } else {
        assert(childJson != nullptr);
        verdad = false;
      }
    }
    {
      const NearTox::BasicJSON *childJson = json.at("wstring");
      if(childJson && childJson->isCompat(NearTox::JS_Str)) {
        Wstring = NearTox::LToString(childJson->GetString());
      } else {
        assert(childJson != nullptr);
        verdad = false;
      }
    }
    {
      const NearTox::BasicJSON *childJson = json.at("myArray");
      if(childJson && childJson->isCompat(NearTox::JS_Array)) {
        for(size_t i = 0; i < childJson->size(); ++i) {
          const NearTox::BasicJSON *sub_childJson = childJson->at(i);
          std::string NewValue;
          {
            if(sub_childJson && sub_childJson->isCompat(NearTox::JS_Str)) {
              NewValue = sub_childJson->GetString();
            } else {
              assert(sub_childJson != nullptr);
              verdad = false;
            }
          }
          MyArray.push_back(std::move(NewValue));
        }
      } else {
        assert(childJson != nullptr);
        verdad = false;
      }
    }
    {
      const NearTox::BasicJSON *childJson = json.at("myWArray");
      if(childJson && childJson->isCompat(NearTox::JS_Array)) {
        for(size_t i = 0; i < childJson->size(); ++i) {
          const NearTox::BasicJSON *sub_childJson = childJson->at(i);
          std::wstring NewValue;
          {
            if(sub_childJson && sub_childJson->isCompat(NearTox::JS_Str)) {
              NewValue = NearTox::LToString(sub_childJson->GetString());
            } else {
              assert(sub_childJson != nullptr);
              verdad = false;
            }
          }
          MyWArray.push_back(std::move(NewValue));
        }
      } else {
        assert(childJson != nullptr);
        verdad = false;
      }
    }
    return verdad;
  }
};
