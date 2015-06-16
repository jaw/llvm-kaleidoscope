class ast_for_expr : public ast_expr {
  vsx_string<> VarName;
  ast_expr *Start, *End, *Step, *Body;

public:
  ast_for_expr(const vsx_string<> &varname, ast_expr *start, ast_expr *end,
             ast_expr *step, ast_expr *body)
      : VarName(varname), Start(start), End(end), Step(step), Body(body) {}

  void dump(vsx_string<char> &out, int ind) override
  {
    out += vsx_string<>("for");
    ast_expr::dump(out, ind);
    vsx_string<> out2;

    out2 = indent(out, ind) + "Cond:";
    Start->dump(out2, ind + 1);

    out2 = indent(out, ind) + "End:";
    End->dump(out2, ind + 1);

    out2 = indent(out, ind) + "Step:";
    Step->dump(out2, ind + 1);

    out2 = indent(out, ind) + "Body:";
    Body->dump(out2, ind + 1);
  }

  llvm::Value *Codegen() override
  {
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
};

