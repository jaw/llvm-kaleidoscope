

/// ast_number_expr - Expression class for numeric literals like "1.0".
class ast_number_expr :
    public ast_expr
{
  double Val;

public:
  ast_number_expr(double val) : Val(val) {}

  void dump(vsx_string<char> &out, int ind)
  {
    out += vsx_string_helper::f2s(Val);
    ast_expr::dump(out, ind);
  }

  llvm::Value *Codegen()
  {
    debug_manager::get_instance()->emitLocation(this);
    return llvm::ConstantFP::get( llvm::getGlobalContext(), llvm::APFloat(Val) );
  }

};
