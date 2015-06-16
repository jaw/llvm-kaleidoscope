#ifndef VX_DEBUG_H
#define VX_DEBUG_H

#include "llvm_includes.h"
#include "debuginfo_abs.h"

using namespace llvm;

class debug_info
    : public debug_abs
{
  llvm::DICompileUnit *TheCU;
  llvm::IRBuilder<>* IBuilder;
  llvm::DIBuilder* DBuilder;
  llvm::DIType DblTy;
  std::vector<llvm::DIScope *> LexicalBlocks;
  std::map<const PrototypeAST *, llvm::DIScope *> FnScopeMap;

public:

  void setIRBuilder(llvm::IRBuilder<>* n)
  {
    IBuilder = n;
  }

  void setDBuilder(llvm::DIBuilder* n)
  {
    DBuilder = n;
  }

  void init()
  {
    TheCU = new DICompileUnit();
    *TheCU = DBuilder->createCompileUnit(
        dwarf::DW_LANG_C, "fib.ks", ".", "Kaleidoscope Compiler", 0, "", 0);
  }

  llvm::DICompileUnit* getCU()
  {
    return TheCU;
  }

  void addFunctionScopeMap(PrototypeAST* proto, llvm::DIScope* scope)
  {
    FnScopeMap[proto] = scope;
  }

  void addFunctionScopeToLexicalBlocks(PrototypeAST* proto)
  {
    LexicalBlocks.push_back(FnScopeMap[proto]);
  }

  void emitLocation(ExprAST *AST)
  {
    if (!AST)
      return IBuilder->SetCurrentDebugLocation(DebugLoc());
    DIScope *Scope;
    if (LexicalBlocks.empty())
      Scope = TheCU;
    else
      Scope = LexicalBlocks.back();
    IBuilder->SetCurrentDebugLocation(
    DebugLoc::get(AST->getLine(), AST->getCol(), *Scope));
  }

  DIType *getDoubleTy()
  {
    if (DblTy)
      return &DblTy;

    DblTy = DBuilder->createBasicType("double", 64, 64, dwarf::DW_ATE_float);
    return &DblTy;
  }

  std::vector<llvm::DIScope *>* getLexicalBlocks()
  {
    return &LexicalBlocks;
  }

  DISubroutineType *CreateFunctionType(unsigned NumArgs, DIFile *Unit)
  {
    SmallVector<Metadata *, 8> EltTys;
    DIType *DblTy = getDoubleTy();

    // Add the result type.
    EltTys.push_back(*DblTy);

    for (unsigned i = 0, e = NumArgs; i != e; ++i)
      EltTys.push_back(*DblTy);

    DISubroutineType* dt = new DISubroutineType;
    *dt = DBuilder->createSubroutineType(*Unit,
                                            DBuilder->getOrCreateTypeArray(EltTys));

    return dt;
  }


};





#endif
