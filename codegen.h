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


llvm::Value *VariableExprAST::Codegen() {
  // Look this variable up in the function.
  llvm::Value *V = named_values::get_instance()->get(Name);
  if (V == 0)
    return ErrorV("Unknown variable name");

  debug_manager::get_instance()->emitLocation(this);
  // Load the value.
  return builder_manager::get_instance()->get_ir()->CreateLoad(V, Name.c_str());
}

llvm::Value *UnaryExprAST::Codegen() {
  llvm::Value *OperandV = Operand->Codegen();
  if (OperandV == 0)
    return 0;

  llvm::Function *F = module_manager::get_instance()->get()->getFunction(std::string("unary") + Opcode);
  if (F == 0)
    return ErrorV("Unknown unary operator");

  debug_manager::get_instance()->emitLocation(this);
  return builder_manager::get_instance()->get_ir()->CreateCall(F, OperandV, "unop");
}

llvm::Value *BinaryExprAST::Codegen() {
  debug_manager::get_instance()->emitLocation(this);

  // Special case '=' because we don't want to emit the LHS as an expression.
  if (Op == '=') {
    // Assignment requires the LHS to be an identifier.
    // This assume we're building without RTTI because LLVM builds that way by
    // default.  If you build LLVM with RTTI this can be changed to a
    // dynamic_cast for automatic error checking.
    VariableExprAST *LHSE = static_cast<VariableExprAST *>(LHS);
    if (!LHSE)
      return ErrorV("destination of '=' must be a variable");
    // Codegen the RHS.
    llvm::Value *Val = RHS->Codegen();
    if (Val == 0)
      return 0;

    // Look up the name.
    llvm::Value *Variable = named_values::get_instance()->get( LHSE->getName() );
    if (Variable == 0)
      return ErrorV("Unknown variable name");

    builder_manager::get_instance()->get_ir()->CreateStore(Val, Variable);
    return Val;
  }

  llvm::Value *L = LHS->Codegen();
  llvm::Value *R = RHS->Codegen();
  if (L == 0 || R == 0)
    return 0;

  switch (Op) {
  case '+':
    return builder_manager::get_instance()->get_ir()->CreateFAdd(L, R, "addtmp");
  case '-':
    return builder_manager::get_instance()->get_ir()->CreateFSub(L, R, "subtmp");
  case '*':
    return builder_manager::get_instance()->get_ir()->CreateFMul(L, R, "multmp");
  case '<':
    L = builder_manager::get_instance()->get_ir()->CreateFCmpULT(L, R, "cmptmp");
    // Convert bool 0/1 to double 0.0 or 1.0
    return
      builder_manager::get_instance()->get_ir()->CreateUIToFP(
        L,
        llvm::Type::getDoubleTy( llvm::getGlobalContext()),
        "booltmp"
      );
  default:
    break;
  }

  // If it wasn't a builtin binary operator, it must be a user defined one. Emit
  // a call to it.
  llvm::Function *F = module_manager::get_instance()->get()->getFunction(std::string("binary") + Op);
  assert(F && "binary operator not found!");

  llvm::Value *Ops[] = { L, R };
  return builder_manager::get_instance()->get_ir()->CreateCall(F, Ops, "binop");
}

llvm::Value *CallExprAST::Codegen() {
  debug_manager::get_instance()->emitLocation(this);

  // Look up the name in the global module table.
  llvm::Function *CalleeF = module_manager::get_instance()->get()->getFunction( std::string(Callee.c_str()) );
  if (CalleeF == 0)
    return ErrorV("Unknown function referenced");

  // If argument mismatch error.
  if (CalleeF->arg_size() != Args.size())
    return ErrorV("Incorrect # arguments passed");

  std::vector< llvm::Value *> ArgsV;
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    ArgsV.push_back(Args[i]->Codegen());
    if (ArgsV.back() == 0)
      return 0;
  }

  return builder_manager::get_instance()->get_ir()->CreateCall(CalleeF, ArgsV, "calltmp");
}

llvm::Value *IfExprAST::Codegen() {
  debug_manager::get_instance()->emitLocation(this);

  llvm::Value *CondV = Cond->Codegen();
  if (CondV == 0)
    return 0;

  // Convert condition to a bool by comparing equal to 0.0.
  CondV = builder_manager::get_instance()->get_ir()->CreateFCmpONE(
      CondV, llvm::ConstantFP::get( llvm::getGlobalContext(), llvm::APFloat(0.0)), "ifcond");

  llvm::Function *TheFunction = builder_manager::get_instance()->get_ir()->GetInsertBlock()->getParent();

  // Create blocks for the then and else cases.  Insert the 'then' block at the
  // end of the function.
  llvm::BasicBlock *ThenBB =
      llvm::BasicBlock::Create( llvm::getGlobalContext(), "then", TheFunction);
  llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create( llvm::getGlobalContext(), "else");
  llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create( llvm::getGlobalContext(), "ifcont");

  builder_manager::get_instance()->get_ir()->CreateCondBr(CondV, ThenBB, ElseBB);

  // Emit then value.
  builder_manager::get_instance()->get_ir()->SetInsertPoint(ThenBB);

  llvm::Value *ThenV = Then->Codegen();
  if (ThenV == 0)
    return 0;

  builder_manager::get_instance()->get_ir()->CreateBr(MergeBB);
  // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
  ThenBB = builder_manager::get_instance()->get_ir()->GetInsertBlock();

  // Emit else block.
  TheFunction->getBasicBlockList().push_back(ElseBB);
  builder_manager::get_instance()->get_ir()->SetInsertPoint(ElseBB);

  llvm::Value *ElseV = Else->Codegen();
  if (ElseV == 0)
    return 0;

  builder_manager::get_instance()->get_ir()->CreateBr(MergeBB);
  // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
  ElseBB = builder_manager::get_instance()->get_ir()->GetInsertBlock();

  // Emit merge block.
  TheFunction->getBasicBlockList().push_back(MergeBB);
  builder_manager::get_instance()->get_ir()->SetInsertPoint(MergeBB);
  llvm::PHINode *PN =
      builder_manager::get_instance()->get_ir()->CreatePHI( llvm::Type::getDoubleTy( llvm::getGlobalContext()), 2, "iftmp");

  PN->addIncoming(ThenV, ThenBB);
  PN->addIncoming(ElseV, ElseBB);
  return PN;
}

llvm::Value *ForExprAST::Codegen() {
  // Output this as:
  //   var = alloca double
  //   ...
  //   start = startexpr
  //   store start -> var
  //   goto loop
  // loop:
  //   ...
  //   bodyexpr
  //   ...
  // loopend:
  //   step = stepexpr
  //   endcond = endexpr
  //
  //   curvar = load var
  //   nextvar = curvar + step
  //   store nextvar -> var
  //   br endcond, loop, endloop
  // outloop:

  llvm::Function *TheFunction = builder_manager::get_instance()->get_ir()->GetInsertBlock()->getParent();

  // Create an alloca for the variable in the entry block.
  llvm::AllocaInst *Alloca = llvm_helper::CreateEntryBlockAlloca(TheFunction, std::string(VarName.c_str()));

  debug_manager::get_instance()->emitLocation(this);

  // Emit the start code first, without 'variable' in scope.
  llvm::Value *StartVal = Start->Codegen();
  if (StartVal == 0)
    return 0;

  // Store the value into the alloca.
  builder_manager::get_instance()->get_ir()->CreateStore(StartVal, Alloca);

  // Make the new basic block for the loop header, inserting after current
  // block.
  llvm::BasicBlock *LoopBB =
      llvm::BasicBlock::Create( llvm::getGlobalContext(), "loop", TheFunction);

  // Insert an explicit fall through from the current block to the LoopBB.
  builder_manager::get_instance()->get_ir()->CreateBr(LoopBB);

  // Start insertion in LoopBB.
  builder_manager::get_instance()->get_ir()->SetInsertPoint(LoopBB);

  // Within the loop, the variable is defined equal to the PHI node.  If it
  // shadows an existing variable, we have to restore it, so save it now.
  llvm::AllocaInst *OldVal = named_values::get_instance()->get( VarName );
  named_values::get_instance()->set( VarName, Alloca );

  // Emit the body of the loop.  This, like any other expr, can change the
  // current BB.  Note that we ignore the value computed by the body, but don't
  // allow an error.
  if (Body->Codegen() == 0)
    return 0;

  // Emit the step value.
  llvm::Value *StepVal;
  if (Step) {
    StepVal = Step->Codegen();
    if (StepVal == 0)
      return 0;
  } else {
    // If not specified, use 1.0.
    StepVal = llvm::ConstantFP::get( llvm::getGlobalContext(), llvm::APFloat(1.0));
  }

  // Compute the end condition.
  llvm::Value *EndCond = End->Codegen();
  if (EndCond == 0)
    return EndCond;

  // Reload, increment, and restore the alloca.  This handles the case where
  // the body of the loop mutates the variable.
  llvm::Value *CurVar = builder_manager::get_instance()->get_ir()->CreateLoad(Alloca, VarName.c_str());
  llvm::Value *NextVar = builder_manager::get_instance()->get_ir()->CreateFAdd(CurVar, StepVal, "nextvar");
  builder_manager::get_instance()->get_ir()->CreateStore(NextVar, Alloca);

  // Convert condition to a bool by comparing equal to 0.0.
  EndCond = builder_manager::get_instance()->get_ir()->CreateFCmpONE(
      EndCond, llvm::ConstantFP::get( llvm::getGlobalContext(), llvm::APFloat(0.0)), "loopcond");

  // Create the "after loop" block and insert it.
  llvm::BasicBlock *AfterBB =
      llvm::BasicBlock::Create( llvm::getGlobalContext(), "afterloop", TheFunction);

  // Insert the conditional branch into the end of LoopEndBB.
  builder_manager::get_instance()->get_ir()->CreateCondBr(EndCond, LoopBB, AfterBB);

  // Any new code will be inserted in AfterBB.
  builder_manager::get_instance()->get_ir()->SetInsertPoint(AfterBB);

  // Restore the unshadowed variable.
  if (OldVal)
    named_values::get_instance()->set( VarName, OldVal );
  else
    named_values::get_instance()->unset( VarName );

  // for expr always returns 0.0.
  return llvm::Constant::getNullValue( llvm::Type::getDoubleTy( llvm::getGlobalContext() ) );
}

llvm::Value *VarExprAST::Codegen() {
  std::vector< llvm::AllocaInst *> OldBindings;

  llvm::Function *TheFunction = builder_manager::get_instance()->get_ir()->GetInsertBlock()->getParent();

  // Register all variables and emit their initializer.
  for (unsigned i = 0, e = VarNames.size(); i != e; ++i) {
    const vsx_string<> &VarName = VarNames[i].first;
    ExprAST *Init = VarNames[i].second;

    // Emit the initializer before adding the variable to scope, this prevents
    // the initializer from referencing the variable itself, and permits stuff
    // like this:
    //  var a = 1 in
    //    var a = a in ...   # refers to outer 'a'.
    llvm::Value *InitVal;
    if (Init) {
      InitVal = Init->Codegen();
      if (InitVal == 0)
        return 0;
    } else { // If not specified, use 0.0.
      InitVal = llvm::ConstantFP::get( llvm::getGlobalContext(), llvm::APFloat(0.0));
    }

    llvm::AllocaInst *Alloca = llvm_helper::CreateEntryBlockAlloca(TheFunction, std::string(VarName.c_str()) );
    builder_manager::get_instance()->get_ir()->CreateStore(InitVal, Alloca);

    // Remember the old variable binding so that we can restore the binding when
    // we unrecurse.
    OldBindings.push_back( named_values::get_instance()->get( VarName ) );

    // Remember this binding.
    named_values::get_instance()->set( VarName, Alloca );
  }

  debug_manager::get_instance()->emitLocation(this);

  // Codegen the body, now that all vars are in scope.
  llvm::Value *BodyVal = Body->Codegen();
  if (BodyVal == 0)
    return 0;

  // Pop all our variables from scope.
  for (unsigned i = 0, e = VarNames.size(); i != e; ++i)
    named_values::get_instance()->set( VarNames[i].first, OldBindings[i] );

  // Return the body computation.
  return BodyVal;
}



