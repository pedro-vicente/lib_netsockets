#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "socket.hh"

///////////////////////////////////////////////////////////////////////////////////////
//main
//example of a TCP client that writes a message with end of message format "\r\n"
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  char buf[10];
  sprintf(buf, "12345");
  tcp_client_t client("127.0.0.1", 2000);
  //create socket and open connection
  client.open();
  client.write_all(buf, strlen(buf));
  std::cout << "client sent " << strlen(buf) << " bytes: " << buf;
  //close connection
  client.close_socket();
  return 0;
}
