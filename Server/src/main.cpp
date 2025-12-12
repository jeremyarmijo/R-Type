#include "include/ServerGame.hpp"

int main() {
  ServerGame server;

  if (!server.Initialize(4242, 4243)) {
    return 1;
  }

  server.Run();
  server.Shutdown();

  return 0;
}
