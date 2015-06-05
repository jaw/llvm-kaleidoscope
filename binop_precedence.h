#ifndef BINOP_PRECEDENCE_H
#define BINOP_PRECEDENCE_H


class binop
{
public:
  static int getBinopPrecedence(char p)
  {
    if (p == '=')
      return 2;
    if (p == '<')
      return 10;
    if (p == '+')
      return 20;
    if (p == '-')
      return 20;
    if (p == '*')
      return 40;

    return 10000;
  }

};



#endif
