#ifndef VX_AST_EXPR_H
#define VX_AST_EXPR_H

#include "llvm_includes.h"
#include "parse.h"


/// ExprAST - Base class for all expression nodes.
class ExprAST {
  SourceLocation Loc;

public:

  int getLine() const
  {
    return Loc.Line;
  }

  int getCol() const
  {
    return Loc.Col;
  }

  ExprAST(SourceLocation Loc = CurLoc) : Loc(Loc)
  {

  }

  virtual void dump(vsx_string<char> &out, int ind)
  {
    out += ":" + vsx_string_helper::i2s(getLine()) + ":" + vsx_string_helper::i2s(getCol()) + "\n";
  }

  virtual ~ExprAST() {}
  virtual llvm::Value* Codegen() = 0;
};


#endif
