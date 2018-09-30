#include "tls_socket.hh"

int main(int argc, char *argv[])
{
  tls_socket_t ssl;
  ssl.open("api.exchangeratesapi.io");
  std::string request =
    "GET /latest?symbols=USD HTTP/1.1\r\nHost: api.exchangeratesapi.io\r\nConnection: close\r\n\r\n";
  ssl.send(request.c_str());
  ssl.receive();
  return 0;
}
