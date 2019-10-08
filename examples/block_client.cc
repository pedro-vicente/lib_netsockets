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
//-s 127.0.0.1 -n 3
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  std::string host_name;
  size_t nbr_itr = 2;
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
      case 'n':
        nbr_itr = std::stoi(argv[i + 1]);
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
  char buf_s[20];
  char buf_r[20];
  sprintf(buf_s, "1234567890");
  for (int idx = 0; idx < nbr_itr; idx++)
  {
    wait(2);

    //create socket and open connection
    client.connect();
    std::cout << "client connected  " << client.m_sockfd << "\n";

    //write something
    client.write_all(buf_s, strlen(buf_s));
    std::cout << "client sent " << strlen(buf_s) << " bytes: " << buf_s
      << ", " << idx + 1 << "/" << nbr_itr << "\n";

    //close connection (server must read all)
    client.close();
    std::cout << "client closed " << "\n";

    //create socket and open connection
    client.connect();
    std::cout << "client connected  " << client.m_sockfd << "\n";

    //read
    int size = client.read_all(buf_r, 10);
    std::string str(buf_r, size);
    std::cout << "client received " << size << " bytes: " << str.c_str() << "\n";

    //close connection
    client.close();
    std::cout << "client closed " << "\n";
  }
  return 0;
}
