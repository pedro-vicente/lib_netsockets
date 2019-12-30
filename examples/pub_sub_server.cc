#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "socket.hh"

void usage()
{
  std::cout << "usage: /pub_sub_server <-p port>" << std::endl;
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//main
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  char buf[255];
  std::ofstream ofs;
  size_t size_recv = 0;
  size_t nbr_recv = 0;
  unsigned short server_port = 4000;

  for (int i = 1; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      switch (argv[i][1])
      {
      case 'p':
        server_port = std::stoi(argv[i + 1]);
        i++;
        break;
      default:
        usage();
      }
    }
  }

  ofs.open("log.txt", std::ofstream::out);
  ofs.close();

  tcp_server_t server(server_port);
  std::cout << "server listening\n";

  while (true)
  {
    socket_t socket = server.accept();
    char* str_ip = inet_ntoa(socket.m_sockaddr_in.sin_addr);

    int size = socket.read_all(buf, sizeof(buf));
    nbr_recv++;
    size_recv += size;
    std::string str(buf, size);
    std::cout << nbr_recv << " " << size << " ";
    std::cout << prt_time() << "," << str_ip << "," << socket.m_sockfd << "\n";

    if (1)
    {
      ofs.open("log.txt", std::ofstream::out | std::ofstream::app);
      str = prt_time();
      str += ",";
      str += str_ip;
      str += ",";
      str += std::to_string(size_recv);
      str += ",";
      str += std::to_string(nbr_recv);
      str += "\n";
      ofs.write(str.c_str(), str.size());
      ofs.close();
    }

    //close connection
    socket.close();
  }
  server.close();

  return 0;
}

