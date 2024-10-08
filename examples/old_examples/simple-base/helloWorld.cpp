#include <iostream>

#include "/home/22m1526/minotaur/src/base/Environment.h"

// Simplest example using the base library. Just prints the version number
// of Minotaur.
int main()
{
  Minotaur::EnvPtr env(new Minotaur::Environment());

  // Write full version.
  env->writeFullVersion(std::cout);

  // Greetings.
  std::cout << std::endl << "Hello World" << std::endl;

  return 0;
}

