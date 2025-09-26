#include "lib/wasm.h"
#include "lib/game.h"

#include <iostream>

int main(int argc, char **argv)
{

  std::cout << "Wasm Scout Initilized" << std::endl;

  auto root_state = std::make_unique<scout::GameState>();
  root_state = root_state->move(8)->move(1)->move(7)->move(3)->move(6)->move(3)->move(4)->move(1)->move(8)->move(8);

  infer(*root_state);
  return 0;
}