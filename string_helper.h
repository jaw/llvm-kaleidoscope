#ifndef STRING_HELPER_H
#define STRING_HELPER_H

namespace string_helper
{

  inline std::string i2s(int in)
  {
    char string_res[256] = "";
    sprintf(string_res,"%d",in);
    return std::string(string_res);
  }

  inline std::string f2s(float in)
  {
    char string_res[256] = "";
    sprintf(string_res,"%f",in);
    return std::string(string_res);
  }

}

#endif
