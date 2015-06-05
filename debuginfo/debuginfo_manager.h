#ifndef VX_DEBUG_MANAGER_H
#define VX_DEBUG_MANAGER_H

#include "debuginfo_abs.h"

class debug_manager
{
public:
  static debug_abs* get_instance();
};

#endif
