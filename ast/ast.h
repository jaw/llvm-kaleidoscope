#ifndef VX_AST_H
#define VX_AST_H

#include "debuginfo/debuginfo_manager.h"
#include "ast_expr.h"

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//

vsx_string<> &indent(vsx_string<> &O, int size)
{
  for (size_t i = 0; i < size; i++)
    O += ' ';
  return O;
}


/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST :
    public ExprAST
{
  double Val;

public:
  NumberExprAST(double val) : Val(val) {}

  void dump(vsx_string<char> &out, int ind)
  {
    out += vsx_string_helper::f2s(Val);
    ExprAST::dump(out, ind);
  }

  Value *Codegen()
  {
    debug_manager::get_instance()->emitLocation(this);
    return ConstantFP::get(getGlobalContext(), APFloat(Val));
  }

};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
  vsx_string<char> Name;

public:
  VariableExprAST(SourceLocation Loc, const vsx_string<> &name)
      : ExprAST(Loc), Name(name) {}

  const vsx_string<char> &getName() const
  {
    return Name;
  }

  void dump(vsx_string<char> &out, int ind) override
  {
    out += Name;
    ExprAST::dump(out, ind);
  }

  Value *Codegen() override;
};

/// UnaryExprAST - Expression class for a unary operator.
class UnaryExprAST : public ExprAST {
  char Opcode;
  ExprAST *Operand;

public:
  UnaryExprAST(char opcode, ExprAST *operand)
      :
        Opcode(opcode),
        Operand(operand)
  {}

  void dump(vsx_string<char> &out, int ind) override
  {
    out += Opcode;
    ExprAST::dump(out, ind);
    Operand->dump(out, ind + 1);
  }

  Value *Codegen() override;
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
  char Op;
  ExprAST *LHS, *RHS;

public:
  BinaryExprAST(SourceLocation Loc, char op, ExprAST *lhs, ExprAST *rhs)
      : ExprAST(Loc), Op(op), LHS(lhs), RHS(rhs) {}

  void dump(vsx_string<char> &out, int ind) override
  {
    out += vsx_string<>("binary") + Op;
    ExprAST::dump(out, ind);
    vsx_string<> out2 = indent(out, ind) + "LHS:";
    LHS->dump(out2, ind + 1);
    out2 = indent(out, ind) + "RHS:";
    RHS->dump(out2, ind + 1);
  }
  Value *Codegen() override;
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
  vsx_string<> Callee;
  std::vector<ExprAST *> Args;

public:
  CallExprAST(SourceLocation Loc, const vsx_string<> &callee,
              std::vector<ExprAST *> &args)
      : ExprAST(Loc), Callee(callee), Args(args) {}

  void dump(vsx_string<char> &out, int ind) override
  {
    out += vsx_string<>("call ") + Callee;

    ExprAST::dump(out, ind);
    for (ExprAST *Arg : Args)
    {
      vsx_string<> out2 = indent(out, ind + 1);
      Arg->dump(out2, ind + 1);
    }
  }

  Value *Codegen() override;
};

/// IfExprAST - Expression class for if/then/else.
class IfExprAST : public ExprAST {
  ExprAST *Cond, *Then, *Else;

public:
  IfExprAST(SourceLocation Loc, ExprAST *cond, ExprAST *then, ExprAST *_else)
      : ExprAST(Loc), Cond(cond), Then(then), Else(_else) {}

  void dump(vsx_string<char> &out, int ind) override
  {
    out += vsx_string<>("if");
    ExprAST::dump(out, ind);
    vsx_string<> out2;

    out2 = indent(out, ind) + "Cond:";
    Cond->dump(out2, ind + 1);
    out2 = indent(out, ind) + "Then:";
    Then->dump(out2, ind + 1);
    out2 = indent(out, ind) + "Else:";
    Else->dump(out2, ind + 1);
  }

  Value *Codegen() override;
};

/// ForExprAST - Expression class for for/in.
class ForExprAST : public ExprAST {
  vsx_string<> VarName;
  ExprAST *Start, *End, *Step, *Body;

public:
  ForExprAST(const vsx_string<> &varname, ExprAST *start, ExprAST *end,
             ExprAST *step, ExprAST *body)
      : VarName(varname), Start(start), End(end), Step(step), Body(body) {}

  void dump(vsx_string<char> &out, int ind) override
  {
    out += vsx_string<>("for");
    ExprAST::dump(out, ind);
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

  Value *Codegen() override;
};

/// VarExprAST - Expression class for var/in
class VarExprAST : public ExprAST {
  std::vector<std::pair<vsx_string<>, ExprAST *> > VarNames;
  ExprAST *Body;

public:
  VarExprAST(const std::vector<std::pair<vsx_string<>, ExprAST *> > &varnames,
             ExprAST *body)
      : VarNames(varnames), Body(body) {}

  void dump(vsx_string<char> &out, int ind) override
  {
  out += "var";
    ExprAST::dump(out, ind);
    for (const auto &NamedVar : VarNames)
    {
      out += indent(out, ind) + NamedVar.first + ":";

      NamedVar.second->dump(out, ind + 1);
    }
    out += indent(out, ind) + "Body:";
    Body->dump( out, ind + 1);
  }

  Value *Codegen() override;
};


/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
  PrototypeAST *Proto;
  ExprAST *Body;

public:
  FunctionAST(PrototypeAST *proto, ExprAST *body) : Proto(proto), Body(body) {}

  virtual void dump(vsx_string<char> &out, int ind)
  {
    out += indent(out, ind) + "FunctionAST\n";
    ++ind;
    out += indent(out, ind) + "Body:";
    if (Body)
      Body->dump(out, ind);
    else
      out += "null\n";
  }

  Function *Codegen();
};

#endif
