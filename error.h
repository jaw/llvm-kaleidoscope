#ifndef ERROR_H
#define ERROR_H


class error
{
public:
  static void print(const char *Str)
  {
    fprintf(stderr, "Error: %s\n", Str);
  }
};

#endif
