#include "lib/hello.h"

#include <iostream>

int main(int argc, char **argv)
{
  std::string who = "world";
  if (argc > 1)
  {
    who = argv[1];
  }

  std::cout << scout::get_greet(who) << std::endl;

  // infer();

  return 0;
}