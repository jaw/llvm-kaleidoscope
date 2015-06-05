#include "producer.h"

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
