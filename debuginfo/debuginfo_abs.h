#ifndef VX_DEBUG_ABS_H
#define VX_DEBUG_ABS_H

#include "llvm_includes.h"

#include "ast/ast_abs.h"
#include "ast/ast_expr.h"


class debug_abs
{
public:

  virtual void setIRBuilder(llvm::IRBuilder<>* n) = 0;
  virtual void setDBuilder(llvm::DIBuilder* n) = 0;
  virtual void init() = 0;
  virtual void emitLocation(ExprAST *AST) = 0;
  virtual llvm::DICompileUnit* getCU() = 0;
  virtual llvm::DIType *getDoubleTy() = 0;
  virtual llvm::DISubroutineType *CreateFunctionType(unsigned NumArgs, llvm::DIFile *Unit) = 0;
  virtual std::vector<llvm::DIScope *>* getLexicalBlocks() = 0;
  virtual void addFunctionScopeMap(PrototypeAST* proto, llvm::DIScope* scope) = 0;
  virtual void addFunctionScopeToLexicalBlocks(PrototypeAST* proto) = 0;

};

#endif
