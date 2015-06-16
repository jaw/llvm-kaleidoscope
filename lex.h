#ifndef LEX_H
#define LEX_H

#include "vsx_string.h"
#include "vsx_string_helper.h"


// The lexer returns tokens [0-255] if it is an unknown character, otherwise one
// of these for known things.
enum Token {
  tok_eof = -1,

  // function
  tok_function = -2,

  // commands
  //tok_def = -2,
  tok_extern = -3,

  // primary
  tok_identifier = -4,
  tok_number = -5,

  // control
  tok_if = -6,
  tok_then = -7,
  tok_else = -8,
  tok_for = -9,
  tok_in = -10,

  // operators
  tok_binary = -11,
  tok_unary = -12,

  // var definition
  tok_var = -13

};

/*vsx_string<char> getTokName(int Tok)
{
  switch (Tok)
  {
    case tok_eof:
      return "eof";
    case tok_def:
      return "def";
    case tok_extern:
      return "extern";
    case tok_identifier:
      return "identifier";
    case tok_number:
      return "number";
    case tok_if:
      return "if";
    case tok_then:
      return "then";
    case tok_else:
      return "else";
    case tok_for:
      return "for";
    case tok_in:
      return "in";
    case tok_binary:
      return "binary";
    case tok_unary:
      return "unary";
    case tok_var:
      return "var";
  }

  vsx_string<char> ret;
  ret = vsx_string_helper::i2s( Tok );

  return ret;
}
*/

#endif
