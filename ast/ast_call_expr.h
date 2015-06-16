///  Expression class for function calls.
class ast_call_expr : public ast_expr {
  std::string Callee;
  std::vector<ast_expr *> Args;

public:
  ast_call_expr(SourceLocation Loc, const std::string &callee,
              std::vector<ast_expr *> &args)
      : ast_expr(Loc), Callee(callee), Args(args) {}

  void dump(std::string &out, int ind) override
  {
    out += std::string("call ") + Callee;

    ast_expr::dump(out, ind);
    for (ast_expr *Arg : Args)
    {
      std::string out2 = indent(out, ind + 1);
      Arg->dump(out2, ind + 1);
    }
  }

  llvm::Value *Codegen() override
  {
    debug_manager::get_instance()->emitLocation(this);

    // Look up the name in the global module table.
    llvm::Function *CalleeF = module_manager::get_instance()->get()->getFunction( std::string(Callee.c_str()) );
    if (CalleeF == 0)
    {
      error::print("Unknown function referenced");
      return 0;
    }

    // If argument mismatch error.
    if (CalleeF->arg_size() != Args.size())
    {
      error::print("Incorrect # arguments passed");
      return 0;
    }

    std::vector< llvm::Value *> ArgsV;
    for (unsigned i = 0, e = Args.size(); i != e; ++i) {
      ArgsV.push_back(Args[i]->Codegen());
      if (ArgsV.back() == 0)
        return 0;
    }

    return builder_manager::get_instance()->get_ir()->CreateCall(CalleeF, ArgsV, "calltmp");
  }

};
