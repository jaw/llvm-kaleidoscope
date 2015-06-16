#ifndef LLVM_HELPER_H
#define LLVM_HELPER_H

class llvm_helper
{
public:

  /// CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
  /// the function.  This is used for mutable variables etc.
  static llvm::AllocaInst *CreateEntryBlockAlloca
  (
      llvm::Function *TheFunction,
      const std::string &VarName
  )
  {
    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                     TheFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(llvm::Type::getDoubleTy(llvm::getGlobalContext()), 0,
                             VarName.c_str());
  }

};


#endif
