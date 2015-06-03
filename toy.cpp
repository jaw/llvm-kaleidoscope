#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"
#include <cctype>
#include <cstdio>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace llvm;

size_t program_strpos = 0;
std::string program =
  "extern printd(x)"
  "def fib(x)\n"
  "  if x < 3 then\n"
  "    1\n"
  "  else\n"
  "    fib(x-1)+fib(x-2)\n"
  "\n"
  "fib(40)"
;

char get_prog_char()
{
  char c = program[program_strpos];
  program_strpos++;
  return c;
}

#include "lex.h"
#include "ast.h"
#include "debug.h"
#include "parse.h"
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
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();
  LLVMContext &Context = getGlobalContext();

  // Install standard binary operators.
  // 1 is lowest precedence.
  BinopPrecedence['='] = 2;
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40; // highest.

  // Prime the first token.
  getNextToken();

  // Make the module, which holds all the code.
  std::unique_ptr<Module> Owner = make_unique<Module>("my cool jit", Context);
  TheModule = Owner.get();

  // Add the current debug info version into the module.
  TheModule->addModuleFlag(Module::Warning, "Debug Info Version",
                           DEBUG_METADATA_VERSION);

  // Darwin only supports dwarf2.
  if (Triple(sys::getProcessTriple()).isOSDarwin())
    TheModule->addModuleFlag(llvm::Module::Warning, "Dwarf Version", 2);

  // Construct the DIBuilder, we do this here because we need the module.
  DBuilder = new DIBuilder(*TheModule);

  // Create the compile unit for the module.
  // Currently down as "fib.ks" as a filename since we're redirecting stdin
  // but we'd like actual source locations.
  KSDbgInfo.TheCU = new DICompileUnit();
  *KSDbgInfo.TheCU = DBuilder->createCompileUnit(
      dwarf::DW_LANG_C, "fib.ks", ".", "Kaleidoscope Compiler", 0, "", 0);

  // Create the JIT.  This takes ownership of the module.
  std::string ErrStr;
  TheExecutionEngine =
      EngineBuilder(std::move(Owner))
          .setErrorStr(&ErrStr)
          //.setMCJITMemoryManager(llvm::make_unique<SectionMemoryManager>())
          .create();
  if (!TheExecutionEngine) {
    fprintf(stderr, "Could not create ExecutionEngine: %s\n", ErrStr.c_str());
    exit(1);
  }

  legacy::FunctionPassManager OurFPM(TheModule);

  // Set up the optimizer pipeline.  Start with registering info about how the
  // target lays out data structures.
  TheModule->setDataLayout(TheExecutionEngine->getDataLayout());
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
  DBuilder->finalize();

  // Print out all of the generated code.
  TheModule->dump();

  return 0;
}
