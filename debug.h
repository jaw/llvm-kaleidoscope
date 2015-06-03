//===----------------------------------------------------------------------===//
// Debug Info Support
//===----------------------------------------------------------------------===//

static DIBuilder *DBuilder;

DIType *DebugInfo::getDoubleTy() {
  if (DblTy)
    return &DblTy;

  DblTy = DBuilder->createBasicType("double", 64, 64, dwarf::DW_ATE_float);
  return &DblTy;
}

void DebugInfo::emitLocation(ExprAST *AST) {
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

static DISubroutineType *CreateFunctionType(unsigned NumArgs, DIFile *Unit) {
  SmallVector<Metadata *, 8> EltTys;
  DIType *DblTy = KSDbgInfo.getDoubleTy();

  // Add the result type.
  EltTys.push_back(*DblTy);

  for (unsigned i = 0, e = NumArgs; i != e; ++i)
    EltTys.push_back(*DblTy);

  DISubroutineType* dt = new DISubroutineType;
  *dt = DBuilder->createSubroutineType(*Unit,
                                          DBuilder->getOrCreateTypeArray(EltTys));

  return dt;
}
