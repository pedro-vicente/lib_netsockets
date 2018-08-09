#include <string>
#include <iostream>
#include "openssl_socket.hh"

int main(int argc, char *argv[])
{
  ssl_socket_t ssl;
  ssl.open("finance.google.com");
  std::string request = 
    "GET https://finance.google.com/finance/getprices?i=86400&p=1Y&f=d,o&df=cpct&q=.DJI  HTTP/1.1\r\n\r\n";
  ssl.send(request.c_str());
  ssl.receive();
  ssl.close_socket();
  return 0;
}
