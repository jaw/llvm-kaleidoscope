#ifndef VX_AST_ABS_H
#define VX_AST_ABS_H

#include "llvm_includes.h"
#include "llvm_helper.h"
#include "module_manager.h"
#include "builder_manager.h"
#include "named_values.h"
#include "source_location.h"
#include "error.h"

#include "debuginfo/debuginfo_manager.h"

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its argument names as well as if it is an operator.
class PrototypeAST {
  vsx_string<> Name;
  std::vector<vsx_string<> > Args;
  bool isOperator;
  unsigned Precedence; // Precedence if a binary op.
  int Line;

public:
  PrototypeAST
  (
      SourceLocation Loc,
      const vsx_string<> &name,
      const std::vector<vsx_string<> > &args,
      bool isoperator = false,
      unsigned prec = 0
  )
      :
        Name(name),
        Args(args),
        isOperator(isoperator),
        Precedence(prec),
        Line(Loc.Line)
  {}

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

  const std::vector< vsx_string<> > &getArgs() const { return Args; }
};

#endif
