#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "socket.hh"

///////////////////////////////////////////////////////////////////////////////////////
//main
//example of socket I/O between 2 endpoints
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  char buf[255];
  tcp_server_t server(2000);
  std::ofstream ofs;
  size_t size_recv = 0;
  size_t nbr_recv = 0;
  ofs.open("log.txt", std::ofstream::out);
  ofs.close();
  while (true)
  {
    socket_t socket = server.accept();
    nbr_recv++;

    char* str_ip = inet_ntoa(socket.m_sockaddr_in.sin_addr);
    std::cout << prt_time() << "," << str_ip << "," << socket.m_sockfd << "\n";

    int size = socket.read_all(buf, sizeof(buf));
    size_recv += size;

    std::string str(buf, size);
    std::cout << "received " << size << " bytes: " << str << "\n";

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

    //close connection (client must read all)
    socket.close();

    //send a reply
    sprintf(buf, "67");

    socket = server.accept();
    socket.write_all(buf, strlen(buf));
    std::cout << "server sent " << strlen(buf) << " bytes: " << buf << "\n";

    socket.close();
  }
  server.close();

  return 0;
}



