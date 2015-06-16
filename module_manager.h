#ifndef MODULE_MANAGER_H
#define MODULE_MANAGER_H

class module_manager
{
  llvm::Module* module;
public:

  void set(llvm::Module* n)
  {
    module = n;
  }

  llvm::Module* get()
  {
    return module;
  }

  static module_manager* get_instance()
  {
    static module_manager mm;
    return &mm;
  }
};

#endif
