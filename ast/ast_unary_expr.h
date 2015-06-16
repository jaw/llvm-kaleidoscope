/// ast_unary_expr - Expression class for a unary operator.
class ast_unary_expr : public ast_expr {
  char Opcode;
  ast_expr *Operand;

public:
  ast_unary_expr(char opcode, ast_expr *operand)
      :
        Opcode(opcode),
        Operand(operand)
  {}

  void dump(vsx_string<char> &out, int ind) override
  {
    out += Opcode;
    ast_expr::dump(out, ind);
    Operand->dump(out, ind + 1);
  }

  llvm::Value* Codegen() override
  {
    llvm::Value *OperandV = Operand->Codegen();
    if (OperandV == 0)
      return 0;

    llvm::Function *F = module_manager::get_instance()->get()->getFunction(std::string("unary") + Opcode);
    if (F == 0)
    {
      error::print("Unknown unary operator");
      return 0;
    }

    debug_manager::get_instance()->emitLocation(this);
    return builder_manager::get_instance()->get_ir()->CreateCall(F, OperandV, "unop");
  }

};
