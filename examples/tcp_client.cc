#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "socket.hh"

///////////////////////////////////////////////////////////////////////////////////////
//main
//example of socket I/O between 2 endpoints
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  char buf[10];
  sprintf(buf, "12345");
  tcp_client_t client("127.0.0.1", 2000);

  while (1)
  {

    wait(3);

    //create socket and open connection
    client.connect();

    //write something
    client.write_all(buf, strlen(buf));
    std::cout << "client sent " << strlen(buf) << " bytes: " << buf << "\n";

    //close connection (server must read all)
    client.close_socket();

    //create socket and open connection
    client.connect();

    //read
    int size = client.read_all(buf, strlen(buf));
    std::string str(buf, size);
    std::cout << "client received " << size << " bytes: " << str.c_str() << "\n";

    //close connection
    client.close_socket();
  }
  return 0;
}
