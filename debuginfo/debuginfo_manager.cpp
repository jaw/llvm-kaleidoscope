#include <cctype>
#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "lex.h"
#include "debuginfo.h"
#include "debuginfo_manager.h"


debug_info di;


debug_abs* debug_manager::get_instance()
{
  return (debug_abs*)&di;
}

