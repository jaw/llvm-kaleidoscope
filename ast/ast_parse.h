#ifndef VX_AST_PARSE_H
#define VX_AST_PARSE_H

#include "ast_function_prototype.h"
#include "parser.h"

static ast_expr *ParseExpression();

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
static ast_expr *ParseIdentifierExpr()
{
  std::string IdName = parser::get()->get_identifier();

  SourceLocation LitLoc = parser::get()->get_current_location();

  parser::get()->get_next_token(); // eat identifier.

  if (parser::get()->get_current_token() != '(') // Simple variable ref.
    return new ast_variable_expr(LitLoc, IdName);

  // Call.
  parser::get()->get_next_token(); // eat (
  std::vector<ast_expr *> Args;
  if (parser::get()->get_current_token() != ')') {
    while (1) {
      ast_expr *Arg = ParseExpression();
      if (!Arg)
        return 0;
      Args.push_back(Arg);

      if (parser::get()->get_current_token() == ')')
        break;

      if (parser::get()->get_current_token() != ',')
      {
        error::print("Expected ')' or ',' in argument list");
        return 0;
      }
      parser::get()->get_next_token();
    }
  }

  // Eat the ')'.
  parser::get()->get_next_token();

  return new ast_call_expr(LitLoc, IdName, Args);
}

/// numberexpr ::= number
static ast_expr *ParseNumberExpr() {
  ast_expr *Result = new ast_number_expr(parser::get()->get_number_value());
  parser::get()->get_next_token(); // consume the number
  return Result;
}

/// parenexpr ::= '(' expression ')'
static ast_expr *ParseParenExpr() {
  parser::get()->get_next_token(); // eat (.
  ast_expr *V = ParseExpression();
  if (!V)
    return 0;

  if (parser::get()->get_current_token() != ')')
  {
    error::print("expected ')'");
    return 0;
  }
  parser::get()->get_next_token(); // eat ).
  return V;
}

/// ifexpr ::= 'if' expression 'then' expression 'else' expression
static ast_expr *ParseIfExpr() {
  SourceLocation IfLoc = parser::get()->get_current_location();

  parser::get()->get_next_token(); // eat the if.

  // condition.
  ast_expr *Cond = ParseExpression();
  if (!Cond)
    return 0;

  if (parser::get()->get_current_token() != tok_then)
  {
    error::print("expected then");
    return 0;
  }
  parser::get()->get_next_token(); // eat the then

  ast_expr *Then = ParseExpression();
  if (Then == 0)
    return 0;

  if (parser::get()->get_current_token() != tok_else)
  {
    error::print("expected else");
    return 0;
  }

  parser::get()->get_next_token();

  ast_expr *Else = ParseExpression();
  if (!Else)
    return 0;

  return new ast_if_expr(IfLoc, Cond, Then, Else);
}

/// forexpr ::= 'for' identifier '=' expr ',' expr (',' expr)? 'in' expression
static ast_expr *ParseForExpr() {
  parser::get()->get_next_token(); // eat the for.

  if (parser::get()->get_current_token() != tok_identifier)
  {
    error::print("expected identifier after for");
    return 0;
  }

  std::string IdName = parser::get()->get_identifier();
  parser::get()->get_next_token(); // eat identifier.

  if (parser::get()->get_current_token() != '=')
  {
    error::print("expected '=' after for");
    return 0;
  }
  parser::get()->get_next_token(); // eat '='.

  ast_expr *Start = ParseExpression();
  if (Start == 0)
    return 0;
  if (parser::get()->get_current_token() != ',')
  {
    error::print("expected ',' after for start value");
    return 0;
  }
  parser::get()->get_next_token();

  ast_expr *End = ParseExpression();
  if (End == 0)
    return 0;

  // The step value is optional.
  ast_expr *Step = 0;
  if (parser::get()->get_current_token() == ',') {
    parser::get()->get_next_token();
    Step = ParseExpression();
    if (Step == 0)
      return 0;
  }

  if (parser::get()->get_current_token() != tok_in)
  {
    error::print("expected 'in' after for");
    return 0;
  }
  parser::get()->get_next_token(); // eat 'in'.

  ast_expr *Body = ParseExpression();
  if (Body == 0)
    return 0;

  return new ast_for_expr(IdName, Start, End, Step, Body);
}

/// varexpr ::= 'var' identifier ('=' expression)?
//                    (',' identifier ('=' expression)?)* 'in' expression
static ast_expr *ParseVarExpr() {
  parser::get()->get_next_token(); // eat the var.

  std::vector<std::pair<std::string, ast_expr *> > VarNames;

  // At least one variable name is required.
  if (parser::get()->get_current_token() != tok_identifier)
  {
    error::print("expected identifier after var");
    return 0;
  }

  while (1) {
    std::string Name = parser::get()->get_identifier();
    parser::get()->get_next_token(); // eat identifier.

    // Read the optional initializer.
    ast_expr *Init = 0;
    if (parser::get()->get_current_token() == '=') {
      parser::get()->get_next_token(); // eat the '='.

      Init = ParseExpression();
      if (Init == 0)
        return 0;
    }

    VarNames.push_back(std::make_pair(Name, Init));

    // End of var list, exit loop.
    if (parser::get()->get_current_token() != ',')
      break;
    parser::get()->get_next_token(); // eat the ','.

    if (parser::get()->get_current_token() != tok_identifier)
    {
      error::print("expected identifier list after var");
      return 0;
    }
  }

  // At this point, we have to have 'in'.
  if (parser::get()->get_current_token() != tok_in)
  {
    error::print("expected 'in' keyword after 'var'");
    return 0;
  }
  parser::get()->get_next_token(); // eat 'in'.

  ast_expr *Body = ParseExpression();
  if (Body == 0)
    return 0;

  return new ast_var_expr(VarNames, Body);
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
///   ::= ifexpr
///   ::= forexpr
///   ::= varexpr
static ast_expr *ParsePrimary() {
  switch (parser::get()->get_current_token())
  {
    default:
    {
      error::print("unknown token when expecting an expression");
      return 0;
    }
    case tok_identifier:
      return ParseIdentifierExpr();
    case tok_number:
      return ParseNumberExpr();
    case '(':
      return ParseParenExpr();
    case tok_if:
      return ParseIfExpr();
    case tok_for:
      return ParseForExpr();
    case tok_var:
      return ParseVarExpr();
  }
}

/// unary
///   ::= primary
///   ::= '!' unary
static ast_expr *ParseUnary() {
  // If the current token is not an operator, it must be a primary expr.
  if (!isascii(parser::get()->get_current_token()) || parser::get()->get_current_token() == '(' || parser::get()->get_current_token() == ',')
    return ParsePrimary();

  // If this is a unary operator, read it.
  int Opc = parser::get()->get_current_token();
  parser::get()->get_next_token();
  if (ast_expr *Operand = ParseUnary())
    return new ast_unary_expr(Opc, Operand);
  return 0;
}

/// binoprhs
///   ::= ('+' unary)*
static ast_expr *ParseBinOpRHS(int ExprPrec, ast_expr *LHS) {
  // If this is a binop, find its precedence.
  while (1) {
    int TokPrec = parser::get()->get_token_precedence();

    // If this is a binop that binds at least as tightly as the current binop,
    // consume it, otherwise we are done.
    if (TokPrec < ExprPrec)
      return LHS;

    // Okay, we know this is a binop.
    int BinOp = parser::get()->get_current_token();
    SourceLocation BinLoc = parser::get()->get_current_location();
    parser::get()->get_next_token(); // eat binop

    // Parse the unary expression after the binary operator.
    ast_expr *RHS = ParseUnary();
    if (!RHS)
      return 0;

    // If BinOp binds less tightly with RHS than the operator after RHS, let
    // the pending operator take RHS as its LHS.
    int NextPrec = parser::get()->get_token_precedence();
    if (TokPrec < NextPrec) {
      RHS = ParseBinOpRHS(TokPrec + 1, RHS);
      if (RHS == 0)
        return 0;
    }

    // Merge LHS/RHS.
    LHS = new ast_binary_expr(BinLoc, BinOp, LHS, RHS);
  }
}

/// expression
///   ::= unary binoprhs
///
static ast_expr *ParseExpression() {
  ast_expr *LHS = ParseUnary();
  if (!LHS)
    return 0;

  return ParseBinOpRHS(0, LHS);
}

/// definition ::= 'def' prototype expression
static ast_function *ParseDefinition() {
  parser::get()->get_next_token(); // eat def.
  ast_function_prototype* Proto = ast_function_prototype::parse();
  if (Proto == 0)
    return 0;

  if (ast_expr *E = ParseExpression())
    return new ast_function(Proto, E);
  return 0;
}

/// toplevelexpr ::= expression
static ast_function *ParseTopLevelExpr() {
  SourceLocation FnLoc = parser::get()->get_current_location();
  if (ast_expr *E = ParseExpression()) {
    // Make an anonymous proto.
    ast_function_prototype *Proto =
        new ast_function_prototype(FnLoc, "main", std::vector< std::string >());
    return new ast_function(Proto, E);
  }
  return 0;
}

/// external ::= 'extern' prototype
static ast_function_prototype *ParseExtern() {
  parser::get()->get_next_token(); // eat extern.
  return ast_function_prototype::parse();
}

#endif
