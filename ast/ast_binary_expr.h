/// ast_binary_expr - Expression class for a binary operator.
class ast_binary_expr : public ast_expr
{
  char Op;
  ast_expr *LHS, *RHS;

public:
  ast_binary_expr(SourceLocation Loc, char op, ast_expr *lhs, ast_expr *rhs)
      : ast_expr(Loc), Op(op), LHS(lhs), RHS(rhs) {}

  void dump(std::string &out, int ind) override
  {
    out += std::string("binary") + Op;
    ast_expr::dump(out, ind);
    std::string out2 = indent(out, ind) + "LHS:";
    LHS->dump(out2, ind + 1);
    out2 = indent(out, ind) + "RHS:";
    RHS->dump(out2, ind + 1);
  }
  llvm::Value *Codegen() override
  {
    debug_manager::get_instance()->emitLocation(this);

    // Special case '=' because we don't want to emit the LHS as an expression.
    if (Op == '=') {
      // Assignment requires the LHS to be an identifier.
      // This assume we're building without RTTI because LLVM builds that way by
      // default.  If you build LLVM with RTTI this can be changed to a
      // dynamic_cast for automatic error checking.
      ast_variable_expr *LHSE = static_cast<ast_variable_expr *>(LHS);
      if (!LHSE)
      {
        error::print("destination of '=' must be a variable");
        return 0;
      }
      // Codegen the RHS.
      llvm::Value *Val = RHS->Codegen();
      if (Val == 0)
        return 0;

      // Look up the name.
      llvm::Value *Variable = named_values::get_instance()->get( LHSE->getName() );
      if (Variable == 0)
      {
        error::print("Unknown variable name");
        return 0;
      }

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
};
