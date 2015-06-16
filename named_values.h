#ifndef NAMED_VALUES_H
#define NAMED_VALUES_H

#include "llvm_includes.h"

class named_values
{
  std::map<std::string, llvm::AllocaInst* > values;
public:

  void set(std::string s, llvm::AllocaInst* v)
  {
    values[s] = v;
  }

  llvm::AllocaInst* get(std::string s)
  {
    return values[s];
  }

  void unset(std::string s)
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
