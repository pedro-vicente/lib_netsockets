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
    socket_t socket = server.accept();
    int size = socket.read_all(buf, sizeof(buf));

    std::string str(buf, size);
    std::cout << "server received " << size << " bytes: " << str << "\n";

    //close connection (client must read all)
    socket.close_socket();

    //send a reply
    sprintf(buf, "67");

    socket = server.accept();
    socket.write_all(buf, strlen(buf));
    std::cout << "server sent " << strlen(buf) << " bytes: " << buf << "\n";

    socket.close_socket();
  }
  server.close_socket();
  return 0;
}



