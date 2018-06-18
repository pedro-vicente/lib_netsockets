#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include "socket.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//usage
//File Transfer Access Management Services
/////////////////////////////////////////////////////////////////////////////////////////////////////

bool verbose = true;
int handle_client(socket_t& socket);

void usage()
{
  std::cout << "-p PORT: server port (default 4000)" << std::endl;
  std::cout << "-v: verbose output" << std::endl;
  std::cout << "-h: help, exit" << std::endl;
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//main
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  unsigned short port = 4000;

  for (int i = 1; i < argc && argv[i][0] == '-'; i++)
  {
    switch (argv[i][1])
    {
    case 'h':
      usage();
      break;
    case 'v':
      verbose = true;
      break;
    case 'p':
      port = atoi(argv[i + 1]);
      i++;
      break;
    }
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //server
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  tcp_server_t server(port);

  std::cout << "server: listening on port " << port << std::endl;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //server loop
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  while (true)
  {
    socket_t socket = server.accept_client();

    // convert IP addresses from a dots-and-number string to a struct in_addr
    char *str_ip = inet_ntoa(socket.m_sockaddr_in.sin_addr);
    std::cout << "server: accepted: " << str_ip << "," << socket.m_socket_fd << std::endl;

    if (handle_client(socket) < 0)
    {

    }
    socket.close_socket();
  }
  server.close_socket();
  return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//handle_client
/////////////////////////////////////////////////////////////////////////////////////////////////////

int handle_client(socket_t& socket)
{
  std::string header;

  if (socket.parse_http_headers(header) < 0)
  {
    std::cout << "parse_http_headers error\n";
    return -1;
  }

  std::string method = http_get_method(header);

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //GET method
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  if (method.compare("GET") == 0)
  {
    size_t start = header.find("/");
    size_t end = header.find(" ", start);
    std::string action = header.substr(start + 1, end - start - 1);
    std::cout << "REST method: " << action << "\n";
  }

  return 0;
}

