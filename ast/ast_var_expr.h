/// ast_var_expr - Expression class for var/in
class ast_var_expr : public ast_expr {
  std::vector<std::pair<vsx_string<>, ast_expr *> > VarNames;
  ast_expr *Body;

public:
  ast_var_expr(const std::vector<std::pair<vsx_string<>, ast_expr *> > &varnames,
             ast_expr *body)
      : VarNames(varnames), Body(body) {}

  void dump(vsx_string<char> &out, int ind) override
  {
  out += "var";
    ast_expr::dump(out, ind);
    for (const auto &NamedVar : VarNames)
    {
      out += indent(out, ind) + NamedVar.first + ":";

      NamedVar.second->dump(out, ind + 1);
    }
    out += indent(out, ind) + "Body:";
    Body->dump( out, ind + 1);
  }

  llvm::Value *Codegen() override
  {
    std::vector< llvm::AllocaInst *> OldBindings;

    llvm::Function *TheFunction = builder_manager::get_instance()->get_ir()->GetInsertBlock()->getParent();

    // Register all variables and emit their initializer.
    for (unsigned i = 0, e = VarNames.size(); i != e; ++i) {
      const vsx_string<> &VarName = VarNames[i].first;
      ast_expr *Init = VarNames[i].second;

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

};
