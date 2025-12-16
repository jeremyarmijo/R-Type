#include "include/ServerGame.hpp"

int main(int ac, char **av) {
  ServerGame server;
  int diff = 1;
  if (ac >= 2)
    diff = atoi(av[1]);
  if (diff == 0)
    diff = 1;

  if (!server.Initialize(4242, 4243, diff)) {
    return 1;
  }

  server.Run();
  server.Shutdown();

  return 0;
}
