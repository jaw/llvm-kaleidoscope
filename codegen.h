#include "llvm_includes.h"
#include "llvm_helper.h"
#include "named_values.h"

#include "debuginfo/debuginfo_manager.h"
#include "ast/ast.h"

//===----------------------------------------------------------------------===//
// Code Generation
//===----------------------------------------------------------------------===//


llvm::Value *ErrorV(const char *Str)
{
  error::print(Str);
  return 0;
}






