#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "socket.hh"

void usage()
{
  std::cout << "usage: /tcp_client -s server_ip" << std::endl;
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//main
//example of socket I/O between 2 endpoints
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  std::string host_name;
  for (int i = 1; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      switch (argv[i][1])
      {
      case 's':
        host_name = argv[i + 1];
        i++;
        break;
      default:
        usage();
      }
    }
  }
  if (host_name.empty())
  {
    usage();
  }
  tcp_client_t client(host_name.c_str(), 2000);
  char buf_s[10];
  char buf_r[10];
  sprintf(buf_s, "12345");
  sprintf(buf_r, "\0");
  for (int idx = 0; idx < 10; idx++)
  {
    wait(3);

    //create socket and open connection
    client.connect();

    //write something
    client.write_all(buf_s, strlen(buf_s));
    std::cout << "client sent " << strlen(buf_s) << " bytes: " << buf_s << "\n";

    //close connection (server must read all)
    client.close_socket();

    //create socket and open connection
    client.connect();

    //read
    int size = client.read_all(buf_r, 10);
    std::string str(buf_r, size);
    std::cout << "client received " << size << " bytes: " << str.c_str() << "\n";

    //close connection
    client.close_socket();
  }
  return 0;
}
