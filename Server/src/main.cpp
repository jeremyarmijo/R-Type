#include <string>
#include "include/ServerGame.hpp"

int main(int ac, char **av) {
  ServerGame server;
  int diff = 1;
  std::string host = "0.0.0.0";

  if (ac >= 2) diff = atoi(av[1]);
  if (diff == 0) diff = 1;
  if (ac >= 3) host = av[2];
  if (!server.Initialize(4242, 4243, diff, host)) {
    return 1;
  }
  server.Run();
  server.Shutdown();

  return 0;
}
