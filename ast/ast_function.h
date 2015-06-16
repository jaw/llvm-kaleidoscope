#include "ast_function_prototype.h"
#include "ast_expr.h"

/// ast_function - This class represents a function definition itself.
class ast_function {
  ast_function_prototype *Proto;
  ast_expr *Body;

public:

  ast_function(
    ast_function_prototype *proto,
    ast_expr *body
  )
    :
    Proto(proto),
    Body(body)
  {

  }

  virtual void dump(std::string &out, int ind)
  {
    out += indent(out, ind) + "ast_function\n";
    ++ind;
    out += indent(out, ind) + "Body:";
    if (Body)
      Body->dump(out, ind);
    else
      out += "null\n";
  }

  llvm::Function *Codegen()
  {
    named_values::get_instance()->clear();

    llvm::Function *TheFunction = Proto->Codegen();
    if (TheFunction == 0)
      return 0;

    // Push the current scope.
    debug_manager::get_instance()->addFunctionScopeToLexicalBlocks(Proto);

    // Unset the location for the prologue emission (leading instructions with no
    // location in a function are considered part of the prologue and the debugger
    // will run past them when breaking on a function)
    debug_manager::get_instance()->emitLocation(nullptr);


    // If this is an operator, install it.
    if (Proto->isBinaryOp())
      binop::get_instance()->setPrecedence( Proto->getOperatorName(), Proto->getBinaryPrecedence() );

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *BB = llvm::BasicBlock::Create( llvm::getGlobalContext(), "entry", TheFunction);
    builder_manager::get_instance()->get_ir()->SetInsertPoint(BB);

    // Add all arguments to the symbol table and create their allocas.
    Proto->CreateArgumentAllocas(TheFunction);

    debug_manager::get_instance()->emitLocation(Body);

    if (llvm::Value *RetVal = Body->Codegen()) {
      // Finish off the function.
      builder_manager::get_instance()->get_ir()->CreateRet(RetVal);

      // Pop off the lexical block for the function.
      debug_manager::get_instance()->getLexicalBlocks()->pop_back();

      // Validate the generated code, checking for consistency.
      verifyFunction(*TheFunction);

      // Optimize the function.
      TheFPM->run(*TheFunction);

      return TheFunction;
    }

    // Error reading body, remove function.
    TheFunction->eraseFromParent();

    if (Proto->isBinaryOp())
      binop::get_instance()->removePrecedence( Proto->getOperatorName() );

    // Pop off the lexical block for the function since we added it
    // unconditionally.
    debug_manager::get_instance()->getLexicalBlocks()->pop_back();

    return 0;
  }
};
