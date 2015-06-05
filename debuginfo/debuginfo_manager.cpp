#include <cctype>
#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "lex.h"
#include "debug.h"
#include "debug_manager.h"


debug_abs* get_instance()
{
  static debug_info di;
  return (debug_abs*)&di;
}

