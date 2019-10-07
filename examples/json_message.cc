#define _CRT_NONSTDC_NO_DEPRECATE
#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include "socket.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//write_response
//custom TCP message:
//a header with size in bytes and # terminator
//JSON text
/////////////////////////////////////////////////////////////////////////////////////////////////////

int write_request(socket_t &socket, const char* buf_json)
{
  std::string buf;
  size_t size_json = strlen(buf_json);
  buf = std::to_string(static_cast<long long unsigned int>(size_json));
  buf += "#";
  buf += std::string(buf_json);
  return (socket.write_all(buf.data(), buf.size()));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//read_request
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string read_response(socket_t &socket)
{
  int recv_size; // size in bytes received or -1 on error 
  int size_json = 0; //in bytes
  std::string str_header;
  std::string str;

  //parse header, one character at a time and look for for separator #
  //assume size header lenght less than 20 digits
  for (size_t idx = 0; idx < 20; idx++)
  {
    char c;
    if ((recv_size = ::recv(socket.m_sockfd, &c, 1, 0)) == -1)
    {
      std::cout << "recv error: " << strerror(errno) << std::endl;
      return str;
    }
    if (c == '#')
    {
      break;
    }
    else
    {
      str_header += c;
    }
  }

  //get size
  size_json = static_cast<size_t>(atoi(str_header.c_str()));

  //read from socket with known size
  char *buf = new char[size_json];
  if (socket.read_all(buf, size_json) < 0)
  {
    std::cout << "recv error: " << strerror(errno) << std::endl;
    return str;
  }
  std::string str_json(buf, size_json);
  delete[] buf;
  return str_json;
}



