#include "tls_socket.hh"

int main(int argc, char* argv[])
{
  tls_socket_t ssl;
  ssl.open("127.0.0.1", "8443");
  std::string request =
    "GET /jsetsmanagement/v1/syncservices/beacons HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n";
  ssl.send(request.c_str());
  ssl.receive();
  return 0;
}

int test()
{
  tls_socket_t ssl;
  ssl.open("api.exchangeratesapi.io", "443");
  std::string request =
    "GET /latest?symbols=USD HTTP/1.1\r\nHost: api.exchangeratesapi.io\r\nConnection: close\r\n\r\n";
  ssl.send(request.c_str());
  ssl.receive();
  return 0;
}
