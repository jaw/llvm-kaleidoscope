#ifndef VX_AST_FUNCTION_PROTOTYPE_H
#define VX_AST_FUNCTION_PROTOTYPE_H

#include "llvm_includes.h"
#include "llvm_helper.h"
#include "module_manager.h"
#include "builder_manager.h"
#include "named_values.h"
#include "source_location.h"
#include "error.h"
#include "parser.h"

#include "debuginfo/debuginfo_manager.h"

/// ast_function_prototype - This class represents the "prototype" for a function,
/// which captures its argument names as well as if it is an operator.
class ast_function_prototype
{
  std::string Name;
  std::vector<std::string > Args;
  bool isOperator;
  unsigned Precedence; // Precedence if a binary op.
  int Line;

public:
  ast_function_prototype
  (
      SourceLocation Loc,
      const std::string &name,
      const std::vector<std::string > &args,
      bool isoperator = false,
      unsigned prec = 0
  )
      :
        Name(name),
        Args(args),
        isOperator(isoperator),
        Precedence(prec),
        Line(Loc.Line)
  {

  }

  bool isUnaryOp() const
  {
    return isOperator && Args.size() == 1;
  }

  bool isBinaryOp() const
  {
    return isOperator && Args.size() == 2;
  }

  char getOperatorName() const
  {
    assert(isUnaryOp() || isBinaryOp());
    return Name[Name.size() - 1];
  }

  unsigned getBinaryPrecedence() const
  {
    return Precedence;
  }

  /// prototype
  ///   ::= id '(' id* ')'
  ///   ::= binary LETTER number? (id, id)
  ///   ::= unary LETTER (id)
  static ast_function_prototype* parse()
  {
    std::string FnName;
    SourceLocation FnLoc = parser::get()->get_current_location();

    unsigned Kind = 0; // 0 = identifier, 1 = unary, 2 = binary.
    unsigned BinaryPrecedence = 30;

    switch ( parser::get()->get_current_token() )
    {
    default:
      {
        error::print("Expected function name in prototype");
        return 0;
      }
    case tok_identifier:
      FnName = parser::get()->get_identifier();
      Kind = 0;
      parser::get()->get_next_token();
      break;
    case tok_unary:
      parser::get()->get_next_token();
      if (!isascii( parser::get()->get_current_token() ))
      {
        error::print("Expected unary operator");
        return 0;
      }
      FnName = "unary";
      FnName += (char)parser::get()->get_current_token();
      Kind = 1;
      parser::get()->get_next_token();
      break;
    case tok_binary:
      parser::get()->get_next_token();
      if (!isascii( parser::get()->get_current_token() ))
      {
        error::print("Expected binary operator");
        return 0;
      }
      FnName = "binary";
      FnName += (char)parser::get()->get_current_token();
      Kind = 2;
      parser::get()->get_next_token();

      // Read the precedence if present.
      if (parser::get()->get_current_token() == tok_number) {
        if (parser::get()->get_number_value() < 1 || parser::get()->get_number_value() > 100)
        {
          error::print("Invalid precedecnce: must be 1..100");
          return 0;
        }
        BinaryPrecedence = (unsigned)parser::get()->get_number_value();
        parser::get()->get_next_token();
      }
      break;
    }

    if (parser::get()->get_current_token() != '(')
    {
      error::print("Expected '(' in prototype");
      return 0;
    }

    std::vector<std::string> ArgNames;
    while (parser::get()->get_next_token() == tok_identifier)
      ArgNames.push_back( parser::get()->get_identifier() );
    if (parser::get()->get_current_token() != ')')
    {
      error::print("Expected ')' in prototype");
      return 0;
    }

    // success.
    parser::get()->get_next_token(); // eat ')'.

    // Verify right number of names for operator.
    if (Kind && ArgNames.size() != Kind)
    {
      error::print("Invalid number of operands for operator");
      return 0;
    }

    return new ast_function_prototype(FnLoc, FnName, ArgNames, Kind != 0, BinaryPrecedence);
  }

  llvm::Function* Codegen() {
    // Make the function type:  double(double,double) etc.
    std::vector<llvm::Type *> Doubles(Args.size(),
                                llvm::Type::getDoubleTy(llvm::getGlobalContext()));
    llvm::FunctionType *FT =
        llvm::FunctionType::get(llvm::Type::getDoubleTy(llvm::getGlobalContext()), Doubles, false);

    llvm::Function *F =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, std::string(Name.c_str()), module_manager::get_instance()->get() );

    printf("Func name: %s\n", Name.c_str() );
    fflush(stdout);

    // If F conflicted, there was already something named 'Name'.  If it has a
    // body, don't allow redefinition or reextern.
    if (F->getName() != std::string(Name.c_str())) {
      // Delete the one we just made and get the existing one.
      F->eraseFromParent();
      F = module_manager::get_instance()->get()->getFunction(Name.c_str());


      // If F already has a body, reject this.
      if (!F->empty()) {
        error::print("redefinition of function");
        return 0;
      }

      // If F took a different number of args, reject.
      if (F->arg_size() != Args.size()) {
        error::print("redefinition of function with different # args");
        return 0;
      }
    }

    // Set names for all arguments.
    unsigned Idx = 0;
    for (llvm::Function::arg_iterator AI = F->arg_begin(); Idx != Args.size();
         ++AI, ++Idx)
      AI->setName(Args[Idx].c_str());

    // Create a subprogram DIE for this function.
    llvm::DIFile Unit = builder_manager::get_instance()->get_di()->createFile(
      debug_manager::get_instance()->getCU()->getFilename(),
      debug_manager::get_instance()->getCU()->getDirectory()
    );
    //DIScope *FContext = Unit;
    unsigned LineNo = Line;
    unsigned ScopeLine = Line;
    llvm::DISubprogram* SP = new llvm::DISubprogram;
    *SP = builder_manager::get_instance()->get_di()->createFunction(
        Unit, // ok
        Name.c_str(), // ok
        llvm::StringRef(), // ok
        Unit, // ok
        LineNo, // ok
        *debug_manager::get_instance()->CreateFunctionType(Args.size(), &Unit), // ok
        false /* internal linkage */,
        true /* definition */,
        ScopeLine,
        llvm::DIDescriptor::FlagPrototyped,
        false,
        F
      );

    debug_manager::get_instance()->addFunctionScopeMap(this, SP);
    return F;
  }


  // CreateArgumentAllocas - Create an alloca for each argument and register the
  // argument in the symbol table so that references to it will succeed.
  void CreateArgumentAllocas(llvm::Function *F)
  {
    llvm::Function::arg_iterator AI = F->arg_begin();
    for (unsigned Idx = 0, e = Args.size(); Idx != e; ++Idx, ++AI)
    {
      // Create an alloca for this variable.
      llvm::AllocaInst *Alloca = llvm_helper::CreateEntryBlockAlloca(F, Args[Idx].c_str() );

      // Create a debug descriptor for the variable.
      llvm::DIScope *Scope = debug_manager::get_instance()->getLexicalBlocks()->back();

      auto Unit = builder_manager::get_instance()->get_di()->createFile(
            debug_manager::get_instance()->getCU()->getFilename(),
            debug_manager::get_instance()->getCU()->getDirectory()
          );


      auto D = builder_manager::get_instance()->get_di()->createLocalVariable(
          llvm::dwarf::DW_TAG_arg_variable, *Scope, Args[Idx].c_str(), Unit, Line,
          *debug_manager::get_instance()->getDoubleTy(), Idx);

      builder_manager::get_instance()->get_di()->insertDeclare(
            Alloca,
            D,
            builder_manager::get_instance()->get_di()->createExpression(),
            builder_manager::get_instance()->get_ir()->GetInsertBlock()
            );

      // Store the initial value into the alloca.
      builder_manager::get_instance()->get_ir()->CreateStore(AI, Alloca);

      // Add arguments to variable symbol table.
      named_values::get_instance()->set( Args[Idx], Alloca );
    }
  }

  const std::vector< std::string > &getArgs() const
  {
    return Args;
  }
};

#endif
