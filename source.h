#ifndef SOURCE_H
#define SOURCE_H

class source
{

  vsx_string<> program =
//      "def fib(x)\n"
      "fib (x)\n"
      "  if x < 3 then\n"
      "    1\n"
      "  else\n"
      "    fib(x - 1) + fib(x - 2)\n"
      "\n"
      "fib(40)"
    ;

public:

  vsx_string<>& get()
  {
    return program;
  }

  static source* get_instance()
  {
    static source s;
    return &s;
  }
};

#endif
