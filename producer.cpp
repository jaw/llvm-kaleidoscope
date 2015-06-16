#include <stdio.h>
#include <string>
#include "producer.h"
#include "vsx_string.h"

size_t program_strpos = 0;
std::string program =
  "def fib(x)\n"
  "  if x < 3 then\n"
  "    1\n"
  "  else\n"
  "    fib(x-1)+fib(x-2);\n"
  "\n"
  "fib(40)\n"
;

size_t ps = program.size();

char get_prog_char()
{
  if (program_strpos >= ps)
    return -1;

  char c = program[program_strpos];
//  vsx_string<> s;
//  s.push_back(c);
//  printf("pc: %s\n", s.c_str());
  fflush(stdout);

  program_strpos++;
  return c;
}
