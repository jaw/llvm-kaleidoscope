#include "llvm_includes.h"
#include "module_manager.h"
#include "builder_manager.h"

#include <cctype>
#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <vsx_string.h>

static llvm::ExecutionEngine *TheExecutionEngine;
//static std::map<vsx_string<>, llvm::AllocaInst *> NamedValues;
static llvm::legacy::FunctionPassManager *TheFPM;

#include "lex.h"
#include "parser.h"
#include "ast/ast_function_prototype.h"
#include "ast/ast.h"
#include "error.h"
#include "debuginfo/debuginfo_manager.h"
#include "ast/ast_parse.h"
#include "codegen.h"
#include "dispatch.h"

//===----------------------------------------------------------------------===//
// "Library" functions that can be "extern'd" from user code.
//===----------------------------------------------------------------------===//

/// putchard - putchar that takes a double and returns 0.
extern "C" double putchard(double X) {
  putchar((char)X);
  return 0;
}

/// printd - printf that takes a double prints it as "%f\n", returning 0.
extern "C" double printd(double X) {
  printf("%f\n", X);
  return 0;
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main() {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();
  llvm::LLVMContext &Context = llvm::getGlobalContext();

  // Initialize IR builder
  builder_manager::get_instance()->set_ir( new llvm::IRBuilder<>( llvm::getGlobalContext() ) );

  // Prime the first token.
  parser::get()->get_next_token();

  // Make the module, which holds all the code.
  std::unique_ptr< llvm::Module > Owner = llvm::make_unique< llvm::Module>("my cool jit", Context);

  module_manager::get_instance()->set( Owner.get() );

  // Add the current debug info version into the module.
  module_manager::get_instance()->get()->addModuleFlag( llvm::Module::Warning, "Debug Info Version",
                           llvm::DEBUG_METADATA_VERSION);

  // Darwin only supports dwarf2.
  if (llvm::Triple( llvm::sys::getProcessTriple()).isOSDarwin())
    module_manager::get_instance()->get()->addModuleFlag(llvm::Module::Warning, "Dwarf Version", 2);

  // Construct the DIBuilder, we do this here because we need the module.
  builder_manager::get_instance()->set_di( new llvm::DIBuilder(*module_manager::get_instance()->get()) );

  // Create the compile unit for the module.
  // Currently down as "fib.ks" as a filename since we're redirecting stdin
  // but we'd like actual source locations.
  debug_manager::get_instance()->init();

  // Create the JIT.  This takes ownership of the module.
  std::string ErrStr;
  TheExecutionEngine =
      llvm::EngineBuilder(std::move(Owner))
          .setErrorStr(&ErrStr)
          //.setMCJITMemoryManager(llvm::make_unique<SectionMemoryManager>())
          .create();
  if (!TheExecutionEngine) {
    fprintf(stderr, "Could not create ExecutionEngine: %s\n", ErrStr.c_str());
    exit(1);
  }

  llvm::legacy::FunctionPassManager OurFPM( module_manager::get_instance()->get() );

  // Set up the optimizer pipeline.  Start with registering info about how the
  // target lays out data structures.
  module_manager::get_instance()->get()->setDataLayout(TheExecutionEngine->getDataLayout());
  #if 0
  // Provide basic AliasAnalysis support for GVN.
  OurFPM.add(createBasicAliasAnalysisPass());
  // Promote allocas to registers.
  OurFPM.add(createPromoteMemoryToRegisterPass());
  // Do simple "peephole" optimizations and bit-twiddling optzns.
  OurFPM.add(createInstructionCombiningPass());
  // Reassociate expressions.
  OurFPM.add(createReassociatePass());
  // Eliminate Common SubExpressions.
  OurFPM.add(createGVNPass());
  // Simplify the control flow graph (deleting unreachable blocks, etc).
  OurFPM.add(createCFGSimplificationPass());
  #endif
  OurFPM.doInitialization();

  // Set the global so the code gen can use this.
  TheFPM = &OurFPM;

  // Run the main "interpreter loop" now.
  MainLoop();

  TheFPM = 0;

  // Finalize the debug info.
  builder_manager::get_instance()->get_di()->finalize();

  // Print out all of the generated code.
  module_manager::get_instance()->get()->dump();

  return 0;
}
