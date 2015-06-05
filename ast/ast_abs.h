#ifndef VX_AST_ABS_H
#define VX_AST_ABS_H
/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its argument names as well as if it is an operator.
class PrototypeAST {
  vsx_string<> Name;
  std::vector<vsx_string<> > Args;
  bool isOperator;
  unsigned Precedence; // Precedence if a binary op.
  int Line;

public:
  PrototypeAST(SourceLocation Loc, const vsx_string<> &name,
               const std::vector<vsx_string<> > &args, bool isoperator = false,
               unsigned prec = 0)
      : Name(name), Args(args), isOperator(isoperator), Precedence(prec),
        Line(Loc.Line) {}

  bool isUnaryOp() const { return isOperator && Args.size() == 1; }
  bool isBinaryOp() const { return isOperator && Args.size() == 2; }

  char getOperatorName() const {
    assert(isUnaryOp() || isBinaryOp());
    return Name[Name.size() - 1];
  }

  unsigned getBinaryPrecedence() const { return Precedence; }

  llvm::Function *Codegen();

  void CreateArgumentAllocas(llvm::Function *F);
  const std::vector< vsx_string<> > &getArgs() const { return Args; }
};

#endif
