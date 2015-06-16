#ifndef PARSER_H
#define PARSER_H
#include "producer.h"
#include "lex.h"
#include "binop_precedence.h"
#include "source_location.h"


class parser
{
  /// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
  /// token the parser is looking at.  getNextToken reads another token from the
  /// lexer and updates CurTok with its results.
  int CurTok;
  std::string IdentifierStr; // Filled in if tok_identifier
  double NumVal;             // Filled in if tok_number
  SourceLocation CurLoc;
  SourceLocation LexLoc = { 1, 0 };

public:

  int get_current_token()
  {
    return CurTok;
  }

  std::string& get_identifier()
  {
    return IdentifierStr;
  }

  double get_number_value()
  {
    return NumVal;
  }

  SourceLocation& get_current_location()
  {
    return CurLoc;
  }

  SourceLocation& get_lexer_location()
  {
    return LexLoc;
  }

  int advance()
  {
    int LastChar = get_prog_char();

    if (LastChar == '\n' || LastChar == '\r') {
      LexLoc.Line++;
      LexLoc.Col = 0;
    } else
      LexLoc.Col++;
    return LastChar;
  }


  /// gettok - Return the next token from standard input.
  int get_token()
  {
    static int LastChar = ' ';

    // Skip any whitespace.
    while (isspace(LastChar))
      LastChar = advance();

    CurLoc = LexLoc;

    if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
      IdentifierStr = LastChar;
      while (isalnum((LastChar = advance())))
        IdentifierStr += LastChar;

      if (IdentifierStr == "def")
        return tok_def;
      if (IdentifierStr == "extern")
        return tok_extern;
      if (IdentifierStr == "if")
        return tok_if;
      if (IdentifierStr == "then")
        return tok_then;
      if (IdentifierStr == "else")
        return tok_else;
      if (IdentifierStr == "for")
        return tok_for;
      if (IdentifierStr == "in")
        return tok_in;
      if (IdentifierStr == "binary")
        return tok_binary;
      if (IdentifierStr == "unary")
        return tok_unary;
      if (IdentifierStr == "var")
        return tok_var;
      return tok_identifier;
    }

    if (isdigit(LastChar) || LastChar == '.') { // Number: [0-9.]+
      std::string NumStr;
      do {
        NumStr += LastChar;
        LastChar = advance();
      } while (isdigit(LastChar) || LastChar == '.');

      NumVal = strtod(NumStr.c_str(), 0);
      return tok_number;
    }

    if (LastChar == '#') {
      // Comment until end of line.
      do
        LastChar = advance();
      while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

      if (LastChar != EOF)
        return get_token();
    }

    // Check for end of file.  Don't eat the EOF.
    if (LastChar == EOF)
      return tok_eof;

    // Otherwise, just return the character as its ascii value.
    int ThisChar = LastChar;
    LastChar = advance();
    return ThisChar;
  }

  /// GetTokPrecedence - Get the precedence of the pending binary operator token.
  int get_token_precedence()
  {
    if (!isascii(CurTok))
      return -1;

    // Make sure it's a declared binop.
    int TokPrec = binop::get_instance()->getBinopPrecedence(CurTok);
    if (TokPrec <= 0)
      return -1;
    return TokPrec;
  }


  int get_next_token()
  {
    CurTok = get_token();
    return CurTok;
  }

  static parser* get()
  {
    static parser pp;
    return &pp;
  }
};






#endif
