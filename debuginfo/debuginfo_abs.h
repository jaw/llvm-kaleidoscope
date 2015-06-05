#ifndef VX_DEBUG_ABS_H
#define VX_DEBUG_ABS_H

#include "ast/ast_expr.h"


class debug_abs
{
public:

  virtual void emitLocation(ExprAST *AST) = 0;
  virtual llvm::DICompileUnit* getCU() = 0;
  virtual DISubroutineType *CreateFunctionType(unsigned NumArgs, DIFile *Unit) = 0;
  virtual void addFunctionScopeMap(PrototypeAST* proto, llvm::DIScope* scope) = 0;

};

#endif
