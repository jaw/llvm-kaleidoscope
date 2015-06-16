#ifndef NAMED_VALUES_H
#define NAMED_VALUES_H

#include "llvm_includes.h"
#include "vsx_string.h"

class named_values
{
  std::map<vsx_string<>, llvm::AllocaInst* > values;
public:

  void set(vsx_string<> s, llvm::AllocaInst* v)
  {
    values[s] = v;
  }

  llvm::AllocaInst* get(vsx_string<> s)
  {
    return values[s];
  }

  void unset(vsx_string<> s)
  {
    values.erase(s);
  }

  void clear()
  {
    values.clear();
  }

  static named_values* get_instance()
  {
    static named_values mm;
    return &mm;
  }
};

#endif
