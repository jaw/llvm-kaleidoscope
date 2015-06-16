#ifndef VX_AST_EXPR_H
#define VX_AST_EXPR_H

#include "llvm_includes.h"
#include "parser.h"


/// ast_expr - Base class for all expression nodes.
class ast_expr {
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

  ast_expr(SourceLocation Loc = parser::get()->get_current_location() )
    :
    Loc(Loc)
  {

  }

  virtual void dump(vsx_string<char> &out, int ind)
  {
    out += ":" + vsx_string_helper::i2s(getLine()) + ":" + vsx_string_helper::i2s(getCol()) + "\n";
  }

  virtual ~ast_expr() {}
  virtual llvm::Value* Codegen() = 0;
};


#endif
