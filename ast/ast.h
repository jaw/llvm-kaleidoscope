#ifndef VX_AST_H
#define VX_AST_H

#include "debuginfo/debuginfo_manager.h"
#include "ast_expr.h"
#include "named_values.h"

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//

std::string &indent(std::string &O, int size)
{
  for (size_t i = 0; i < size; i++)
    O += ' ';
  return O;
}


#include "ast_number_expr.h"
#include "ast_variable_expr.h"
#include "ast_unary_expr.h"
#include "ast_binary_expr.h"
#include "ast_call_expr.h"
#include "ast_if_expr.h"
#include "ast_for_expr.h"
#include "ast_var_expr.h"
#include "ast_function.h"


#endif
