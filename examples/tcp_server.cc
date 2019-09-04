#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include "socket.hh"

///////////////////////////////////////////////////////////////////////////////////////
//main
///////////////////////////////////////////////////////////////////////////////////////

void handle_client(socket_t& socket);

int main(int argc, char* argv[])
{
  char buf[255];
  tcp_server_t server(2000);
  while (true)
  {
    socket_t socket = server.accept_client();
    int recv_size = socket.read_all(buf, sizeof(buf));
    std::string str(buf, recv_size);
    std::cout << "server received " << recv_size << " bytes: " << str;
    socket.close_socket();
  }
  server.close_socket();
  return 0;
}



