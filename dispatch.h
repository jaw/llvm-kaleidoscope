
//===----------------------------------------------------------------------===//
// Top-Level parsing and JIT Driver
//===----------------------------------------------------------------------===//

static void HandleDefinition() {
  if (ast_function *F = ParseDefinition()) {
    if (!F->Codegen()) {
      fprintf(stderr, "Error reading function definition:");
    }
  } else {
    // Skip token for error recovery.
    parser::get()->get_next_token();
  }
}

static void HandleExtern() {
  if (ast_function_prototype *P = ParseExtern()) {
    if (!P->Codegen()) {
      fprintf(stderr, "Error reading extern");
    }
  } else {
    // Skip token for error recovery.
    parser::get()->get_next_token();
  }
}

static void HandleTopLevelExpression() {
  // Evaluate a top-level expression into an anonymous function.
  if (ast_function *F = ParseTopLevelExpr()) {
    if (!F->Codegen()) {
      fprintf(stderr, "Error generating code for top level expr\n");
    }
  } else {
    // Skip token for error recovery.
    parser::get()->get_next_token();
  }
}

/// top ::= definition | external | expression | ';'
static void MainLoop() {
  while (1) {
    switch ( parser::get()->get_current_token() )
    {
      case tok_eof:
        return;
      case ';':
        parser::get()->get_next_token();
        break; // ignore top-level semicolons.
      case tok_def:
        HandleDefinition();
        break;
      case tok_extern:
        HandleExtern();
        break;
      default:
        HandleTopLevelExpression();
        break;
    }
  }
}
