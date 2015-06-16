#ifndef BINOP_PRECEDENCE_H
#define BINOP_PRECEDENCE_H


class binop
{

  std::map<char, int> list;

public:

  void setPrecedence(char p, int value)
  {
    list[p] = value;
  }

  void removePrecedence(char p)
  {
    list.erase(p);
  }

  int getBinopPrecedence(char p)
  {
    if (binop::list.find(p) != binop::list.end())
      return binop::list[p];

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

    return -1;
  }

  static binop* get_instance()
  {
    static binop b;
    return &b;
  }

};



#endif
