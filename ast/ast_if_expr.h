/// ast_if_expr - Expression class for if/then/else.
class ast_if_expr : public ast_expr {

  ast_expr* Cond;
  ast_expr* Then;
  ast_expr* Else;

public:

  ast_if_expr(
    SourceLocation Loc,
    ast_expr *cond,
    ast_expr *then,
    ast_expr *_else
  )
      :
        ast_expr(Loc),
        Cond(cond),
        Then(then),
        Else(_else)
  {}

  void dump(vsx_string<char> &out, int ind) override
  {
    out += vsx_string<>("if");
    ast_expr::dump(out, ind);
    vsx_string<> out2;

    out2 = indent(out, ind) + "Cond:";
    Cond->dump(out2, ind + 1);
    out2 = indent(out, ind) + "Then:";
    Then->dump(out2, ind + 1);
    out2 = indent(out, ind) + "Else:";
    Else->dump(out2, ind + 1);
  }

  llvm::Value *Codegen() override
  {
    debug_manager::get_instance()->emitLocation(this);

    llvm::Value *CondV = Cond->Codegen();

    if (CondV == 0)
      return 0;

    // Convert condition to a bool by comparing equal to 0.0.
    CondV = builder_manager::get_instance()->get_ir()->
        CreateFCmpONE(
          CondV,
          llvm::ConstantFP::get(
            llvm::getGlobalContext(),
            llvm::APFloat(0.0)
          ),
          "ifcond"
          );

    llvm::Function *TheFunction = builder_manager::get_instance()->get_ir()->GetInsertBlock()->getParent();

    // Create blocks for the then and else cases.  Insert the 'then' block at the
    // end of the function.
    llvm::BasicBlock *ThenBB  = llvm::BasicBlock::Create( llvm::getGlobalContext(), "then", TheFunction);
    llvm::BasicBlock *ElseBB  = llvm::BasicBlock::Create( llvm::getGlobalContext(), "else");
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
};
