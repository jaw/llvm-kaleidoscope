/// ast_variable_expr - Expression class for referencing a variable, like "a".
class ast_variable_expr : public ast_expr {
  vsx_string<char> Name;

public:
  ast_variable_expr(SourceLocation Loc, const vsx_string<> &name)
      : ast_expr(Loc), Name(name) {}

  const vsx_string<char> &getName() const
  {
    return Name;
  }

  void dump(vsx_string<char> &out, int ind) override
  {
    out += Name;
    ast_expr::dump(out, ind);
  }

  llvm::Value *Codegen() override
  {
    // Look this variable up in the function.
    llvm::Value *V = named_values::get_instance()->get(Name);
    if (V == 0)
    {
      error::print("Unknown variable name");
      return 0;
    }

    debug_manager::get_instance()->emitLocation(this);
    // Load the value.
    return builder_manager::get_instance()->get_ir()->CreateLoad(V, Name.c_str());
  }

};
