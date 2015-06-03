namespace {
class PrototypeAST;
class ExprAST;
}
static IRBuilder<> Builder(getGlobalContext());
struct DebugInfo {
  DICompileUnit *TheCU;
  DIType DblTy;
  std::vector<DIScope *> LexicalBlocks;
  std::map<const PrototypeAST *, DIScope *> FnScopeMap;

  void emitLocation(ExprAST *AST);
  DIType *getDoubleTy();
} KSDbgInfo;

static std::string IdentifierStr; // Filled in if tok_identifier
static double NumVal;             // Filled in if tok_number
struct SourceLocation {
  int Line;
  int Col;
};
static SourceLocation CurLoc;
static SourceLocation LexLoc = { 1, 0 };

static int advance() {
  int LastChar = get_prog_char();

  if (LastChar == '\n' || LastChar == '\r') {
    LexLoc.Line++;
    LexLoc.Col = 0;
  } else
    LexLoc.Col++;
  return LastChar;
}

/// gettok - Return the next token from standard input.
static int gettok() {
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
      return gettok();
  }

  // Check for end of file.  Don't eat the EOF.
  if (LastChar == EOF)
    return tok_eof;

  // Otherwise, just return the character as its ascii value.
  int ThisChar = LastChar;
  LastChar = advance();
  return ThisChar;
}

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//
namespace {

std::ostream &indent(std::ostream &O, int size) {
  return O << std::string(size, ' ');
}

/// ExprAST - Base class for all expression nodes.
class ExprAST {
  SourceLocation Loc;

public:
  int getLine() const { return Loc.Line; }
  int getCol() const { return Loc.Col; }
  ExprAST(SourceLocation Loc = CurLoc) : Loc(Loc) {}
  virtual std::ostream &dump(std::ostream &out, int ind) {
    return out << ':' << getLine() << ':' << getCol() << '\n';
  }
  virtual ~ExprAST() {}
  virtual Value *Codegen() = 0;
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
  double Val;

public:
  NumberExprAST(double val) : Val(val) {}
  std::ostream &dump(std::ostream &out, int ind) override {
    return ExprAST::dump(out << Val, ind);
  }
  Value *Codegen() override;
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
  std::string Name;

public:
  VariableExprAST(SourceLocation Loc, const std::string &name)
      : ExprAST(Loc), Name(name) {}
  const std::string &getName() const { return Name; }
  std::ostream &dump(std::ostream &out, int ind) override {
    return ExprAST::dump(out << Name, ind);
  }
  Value *Codegen() override;
};

/// UnaryExprAST - Expression class for a unary operator.
class UnaryExprAST : public ExprAST {
  char Opcode;
  ExprAST *Operand;

public:
  UnaryExprAST(char opcode, ExprAST *operand)
      : Opcode(opcode), Operand(operand) {}
  std::ostream &dump(std::ostream &out, int ind) override {
    ExprAST::dump(out << "unary" << Opcode, ind);
    Operand->dump(out, ind + 1);
    return out;
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
  std::ostream &dump(std::ostream &out, int ind) override {
    ExprAST::dump(out << "binary" << Op, ind);
    LHS->dump(indent(out, ind) << "LHS:", ind + 1);
    RHS->dump(indent(out, ind) << "RHS:", ind + 1);
    return out;
  }
  Value *Codegen() override;
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
  std::string Callee;
  std::vector<ExprAST *> Args;

public:
  CallExprAST(SourceLocation Loc, const std::string &callee,
              std::vector<ExprAST *> &args)
      : ExprAST(Loc), Callee(callee), Args(args) {}
  std::ostream &dump(std::ostream &out, int ind) override {
    ExprAST::dump(out << "call " << Callee, ind);
    for (ExprAST *Arg : Args)
      Arg->dump(indent(out, ind + 1), ind + 1);
    return out;
  }
  Value *Codegen() override;
};

/// IfExprAST - Expression class for if/then/else.
class IfExprAST : public ExprAST {
  ExprAST *Cond, *Then, *Else;

public:
  IfExprAST(SourceLocation Loc, ExprAST *cond, ExprAST *then, ExprAST *_else)
      : ExprAST(Loc), Cond(cond), Then(then), Else(_else) {}
  std::ostream &dump(std::ostream &out, int ind) override {
    ExprAST::dump(out << "if", ind);
    Cond->dump(indent(out, ind) << "Cond:", ind + 1);
    Then->dump(indent(out, ind) << "Then:", ind + 1);
    Else->dump(indent(out, ind) << "Else:", ind + 1);
    return out;
  }
  Value *Codegen() override;
};

/// ForExprAST - Expression class for for/in.
class ForExprAST : public ExprAST {
  std::string VarName;
  ExprAST *Start, *End, *Step, *Body;

public:
  ForExprAST(const std::string &varname, ExprAST *start, ExprAST *end,
             ExprAST *step, ExprAST *body)
      : VarName(varname), Start(start), End(end), Step(step), Body(body) {}
  std::ostream &dump(std::ostream &out, int ind) override {
    ExprAST::dump(out << "for", ind);
    Start->dump(indent(out, ind) << "Cond:", ind + 1);
    End->dump(indent(out, ind) << "End:", ind + 1);
    Step->dump(indent(out, ind) << "Step:", ind + 1);
    Body->dump(indent(out, ind) << "Body:", ind + 1);
    return out;
  }
  Value *Codegen() override;
};

/// VarExprAST - Expression class for var/in
class VarExprAST : public ExprAST {
  std::vector<std::pair<std::string, ExprAST *> > VarNames;
  ExprAST *Body;

public:
  VarExprAST(const std::vector<std::pair<std::string, ExprAST *> > &varnames,
             ExprAST *body)
      : VarNames(varnames), Body(body) {}

  std::ostream &dump(std::ostream &out, int ind) override {
    ExprAST::dump(out << "var", ind);
    for (const auto &NamedVar : VarNames)
      NamedVar.second->dump(indent(out, ind) << NamedVar.first << ':', ind + 1);
    Body->dump(indent(out, ind) << "Body:", ind + 1);
    return out;
  }
  Value *Codegen() override;
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its argument names as well as if it is an operator.
class PrototypeAST {
  std::string Name;
  std::vector<std::string> Args;
  bool isOperator;
  unsigned Precedence; // Precedence if a binary op.
  int Line;

public:
  PrototypeAST(SourceLocation Loc, const std::string &name,
               const std::vector<std::string> &args, bool isoperator = false,
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

  Function *Codegen();

  void CreateArgumentAllocas(Function *F);
  const std::vector<std::string> &getArgs() const { return Args; }
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
  PrototypeAST *Proto;
  ExprAST *Body;

public:
  FunctionAST(PrototypeAST *proto, ExprAST *body) : Proto(proto), Body(body) {}

  std::ostream &dump(std::ostream &out, int ind) {
    indent(out, ind) << "FunctionAST\n";
    ++ind;
    indent(out, ind) << "Body:";
    return Body ? Body->dump(out, ind) : out << "null\n";
  }

  Function *Codegen();
};
} // end anonymous namespace
