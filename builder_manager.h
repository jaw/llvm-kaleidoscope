#ifndef BUILDER_MANAGER_H
#define BUILDER_MANAGER_H

class builder_manager
{
  llvm::DIBuilder* di_builder;
  llvm::IRBuilder<>* ir_builder;

public:

  void set_di(llvm::DIBuilder* n)
  {
    di_builder = n;
  }

  void set_ir(llvm::IRBuilder<>* n)
  {
    ir_builder = n;
  }

  llvm::DIBuilder* get_di()
  {
    return di_builder;
  }

  llvm::IRBuilder<>* get_ir()
  {
    return ir_builder;
  }

  static builder_manager* get_instance()
  {
    static builder_manager bm;
    return &bm;
  }
};

#endif
