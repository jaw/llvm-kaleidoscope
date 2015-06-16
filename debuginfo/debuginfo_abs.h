#ifndef VX_DEBUG_ABS_H
#define VX_DEBUG_ABS_H

#include "llvm_includes.h"



class debug_abs
{
public:

  virtual void init() = 0;
  virtual void emitLocation(void *AST) = 0;
  virtual llvm::DICompileUnit* getCU() = 0;
  virtual llvm::DIType *getDoubleTy() = 0;
  virtual llvm::DISubroutineType *CreateFunctionType(unsigned NumArgs, llvm::DIFile *Unit) = 0;
  virtual std::vector<llvm::DIScope *>* getLexicalBlocks() = 0;
  virtual void addFunctionScopeMap(void* proto, llvm::DIScope* scope) = 0;
  virtual void addFunctionScopeToLexicalBlocks(void* proto) = 0;

};

#endif
