#ifndef VX_DEBUG_H
#define VX_DEBUG_H

#include "debug_abs.h"

using namespace llvm;

class debug_info
{
  llvm::DICompileUnit *TheCU;
  llvm::DIType DblTy;
  std::vector<llvm::DIScope *> LexicalBlocks;
  std::map<const PrototypeAST *, llvm::DIScope *> FnScopeMap;

public:

  llvm::DICompileUnit* getCU()
  {
    return TheCU;
  }

  void addFunctionScopeMap(PrototypeAST* proto, llvm::DIScope* scope)
  {
    FnScopeMap[proto] = scope;
  }

  void emitLocation(ExprAST *AST)
  {
    if (!AST)
      return Builder.SetCurrentDebugLocation(DebugLoc());
    DIScope *Scope;
    if (LexicalBlocks.empty())
      Scope = TheCU;
    else
      Scope = LexicalBlocks.back();
    Builder.SetCurrentDebugLocation(
    DebugLoc::get(AST->getLine(), AST->getCol(), *Scope));
  }

  DIType *getDoubleTy()
  {
    if (DblTy)
      return &DblTy;

    DblTy = DBuilder->createBasicType("double", 64, 64, dwarf::DW_ATE_float);
    return &DblTy;
  }

  DISubroutineType *CreateFunctionType(unsigned NumArgs, DIFile *Unit)
  {
    SmallVector<Metadata *, 8> EltTys;
    DIType *DblTy = DebugInfo::getInstance()->getDoubleTy();

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
