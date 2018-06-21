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
int send_http_response(socket_t& socket, std::string& msg);

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
  char buf[4096];

  if (socket.parse_http_headers(header) < 0)
  {
    std::cout << "parse_http_headers error\n";
    return -1;
  }

  //parse header
  size_t start = header.find("/");
  size_t end = header.find(" ", start);
  std::string action = header.substr(start + 1, end - start - 1);
  std::cout << "REST method: " << action << "\n";

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //GET method
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::string method = http_get_method(header);
  if (method.compare("GET") == 0)
  {

  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //POST method
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  else if (method.compare("POST") == 0)
  {
    unsigned long long size_body = http_get_field("Content-Length: ", header);
    std::cout << "received: Content-Length: " << size_body << std::endl;
    if (size_body == 0)
    {
      //send response to client
      std::string msg("Invalid request");
      send_http_response(socket, msg);
      return 0;
    }

    //now get body using size of Content-Length
    if (socket.read_all(buf, (int)size_body) < 0)
    {
      std::cout << "recv error: " << strerror(errno) << std::endl;
      return -1;
    }

    buf[size_body] = '\0';
    std::string message(buf);
    std::cout << "received message:" << std::endl;
    std::cout << message;
  }
  else
  {
    std::cout << "invalid HTTP header method: " << method.c_str() << "\n";
    return -1;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //response
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::string resp_buf;
  resp_buf = action;

  std::string response("HTTP/1.1 200 OK\r\n");
  response += "Content-Type: application/json\r\n";
  response += "Content-Length: ";
  response += std::to_string(resp_buf.size());
  response += "\r\n";
  response += "\r\n"; //terminate HTTP headers
  response += resp_buf;
  if (socket.write_all(response.c_str(), response.size()) < 0)
  {
    std::cout << "write response error\n";
  }

  return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//send_http_response 
//send a HTTP message to client
/////////////////////////////////////////////////////////////////////////////////////////////////////

int send_http_response(socket_t& socket, std::string& msg)
{
  //send response to client
  std::string response("HTTP/1.1 200 OK\r\n");
  response += "Content-Length: ";
  response += std::to_string(msg.size());
  response += "\r\n";
  response += "\r\n"; //terminate HTTP
  response += msg;
  if (socket.write_all(response.c_str(), response.size()) < 0)
  {
    std::cout << "write response error\n";
  }
  return 0;
}

