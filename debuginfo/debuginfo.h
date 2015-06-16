#ifndef VX_DEBUG_H
#define VX_DEBUG_H

#include "llvm_includes.h"
#include "debuginfo_abs.h"
#include "ast/ast_function_prototype.h"
#include "ast/ast_expr.h"
#include "builder_manager.h"

using namespace llvm;

class debug_info
    : public debug_abs
{
  llvm::DICompileUnit *TheCU;
  llvm::DIType DblTy;
  std::vector<llvm::DIScope *> LexicalBlocks;
  std::map< const ast_function_prototype *, llvm::DIScope *> FnScopeMap;

public:

  void init()
  {
    TheCU = new DICompileUnit();
    *TheCU = builder_manager::get_instance()->get_di()->createCompileUnit(
        dwarf::DW_LANG_C, "fib.ks", ".", "Kaleidoscope Compiler", 0, "", 0);
  }

  llvm::DICompileUnit* getCU()
  {
    return TheCU;
  }

  void addFunctionScopeMap(void* proto, llvm::DIScope* scope)
  {
    ast_function_prototype* p = static_cast<ast_function_prototype*>(proto);
    if (p)
      FnScopeMap[p] = scope;
  }

  void addFunctionScopeToLexicalBlocks(void* proto)
  {
    ast_function_prototype* p = static_cast<ast_function_prototype*>(proto);
    if (p)
      LexicalBlocks.push_back(FnScopeMap[p]);
  }

  void emitLocation(void *AST)
  {
    if (!AST)
      return builder_manager::get_instance()->get_ir()->SetCurrentDebugLocation(DebugLoc());

    ast_expr* pAST = static_cast<ast_expr*>(AST);


    DIScope *Scope;
    if (LexicalBlocks.empty())
      Scope = TheCU;
    else
      Scope = LexicalBlocks.back();
    builder_manager::get_instance()->get_ir()->SetCurrentDebugLocation(
      DebugLoc::get(pAST->getLine(), pAST->getCol(), *Scope)
    );
  }

  DIType *getDoubleTy()
  {
    if (DblTy)
      return &DblTy;

    DblTy = builder_manager::get_instance()->get_di()->createBasicType("double", 64, 64, dwarf::DW_ATE_float);
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
    *dt = builder_manager::get_instance()->get_di()->createSubroutineType(
          *Unit,
          builder_manager::get_instance()->get_di()->getOrCreateTypeArray(EltTys)
    );

    return dt;
  }


};





#endif
