#if defined (_MSC_VER)
#include <winsock2.h>
#include <ws2tcpip.h>
#endif
#include <iostream>
#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "socket.hh"
#include "http.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//http_client_t::http_client_t()
/////////////////////////////////////////////////////////////////////////////////////////////////////

http_client_t::http_client_t(const char *host_name, const unsigned short server_port)
  : tcp_client_t(host_name, server_port)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//http_client_t::get
/////////////////////////////////////////////////////////////////////////////////////////////////////

int http_client_t::get(const char *path_remote_file)
{
  char buf_request[1024];

  //construct request message using class input parameters
  sprintf(buf_request, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path_remote_file, m_server_ip.c_str());

  //send request, using built in tcp_client_t socket
  if (this->write_all(buf_request, (int)strlen(buf_request)) < 0)
  {
    return -1;
  }

  std::cout << "request: " << buf_request << std::endl;

  //parse headers
  std::string http_headers;
  if (parse_http_headers(m_sockfd, http_headers) < 0)
  {
    return -1;
  }

  unsigned int size_body = (unsigned int)http_get_field("Content-Length: ", http_headers);
  //read from socket with known size
  if (size_body)
  {
    char *buf = new char[size_body];
    if (this->read_all(buf, size_body) < 0)
    {
      std::cout << "recv error: " << strerror(errno) << std::endl;
      return 0;
    }
    std::string str(buf, size_body);
    std::cout << "body:\n" << str << std::endl;
    delete[]buf;
  }
  else
  {
    //receive data and save to local file 
    std::string str_file_name = str_extract(path_remote_file);
    //we sent a close() server request, so we can use the read_all function
    //that checks for recv() return value of zero (connection closed)

    char buf_[4096];
    this->read_all(buf_, sizeof(buf_));
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//http_client_t::post
/////////////////////////////////////////////////////////////////////////////////////////////////////

int http_client_t::post(const std::string& str_body)
{
  char buf[1024];
  //construct request message using class input parameters
  sprintf(buf, "POST / HTTP/1.1\r\nContent-Length: %d\r\nConnection: close\r\n\r\n%s",
    (int)str_body.size(), str_body.c_str());
  //send request, using built in tcp_client_t socket
  if (this->write_all(buf, (int)strlen(buf)) < 0)
  {
    return -1;
  }
  std::cout << "request: " << buf << std::endl;
  return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//parse_http_headers
//return value
//0 nothing received
//1 something received
/////////////////////////////////////////////////////////////////////////////////////////////////////

int parse_http_headers(socketfd_t fd, std::string& header)
{
  int recv_size; // size in bytes received or -1 on error 
  const int size_buf = 4096;
  char buf[size_buf];

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //MSG_PEEK
  //Peeks at an incoming message.The data is treated as unread and the next recv() or similar 
  //function shall still return this data.
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  if ((recv_size = ::recv(fd, buf, size_buf, MSG_PEEK)) == -1)
  {
    std::cout << "recv error: " << strerror(errno) << std::endl;
    return -1;
  }

  if (recv_size == 0)
  {
    return 0;
  }

  std::string str(buf);
  size_t pos = str.find("\r\n\r\n");

  if (pos == std::string::npos)
  {
    std::cout << "HTTP header bad format" << std::endl;
    return -1;
  }

  header = str.substr(0, pos + 4);
  int header_len = static_cast<int>(pos + 4);

  //now get headers with the obtained size from socket
  if ((recv_size = ::recv(fd, buf, header_len, 0)) == -1)
  {
    std::cout << "recv error: " << strerror(errno) << std::endl;
  }

  //sanity check
  std::string str1(buf);
  assert(str1 == str);

  size_t size_header = header.size();
  size_t size_msg = str.size();
  std::cout << "HTTP header size: " << size_header << std::endl;
  std::cout << header.c_str() << std::endl;
  return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//extract a HTTP header field size size and numeric value
//example: extract "Content-Length: 20"
/////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned long long http_get_field(const std::string& str_field, const std::string& str_header)
{
  size_t pos = str_header.find(str_field);
  if (pos == std::string::npos)
  {
    //not found
    return 0;
  }
  pos += std::string(str_field).size();

  //character position
  size_t pos_c = pos;
  //'\r' position
  size_t pos_r;
  while (1)
  {
    pos_r = str_header.find('\r', pos);
    if (pos_r != std::string::npos)
    {
      break;
    }
    pos++;
  }
  //number of digits in number
  size_t size_n = pos_r - pos_c;
  std::string str_n(str_header.substr(pos_c, size_n));
  unsigned long long size_body = std::stoll(str_n);
  return size_body;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//return body part of HTTP response (take out HTPP headers)
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string http_get_body(const std::string& str_header)
{
  std::string body;

  size_t pos = str_header.find("\r\n\r\n");

  if (pos == std::string::npos)
  {
    std::cout << "HTTP header bad format" << std::endl;
    std::cout << str_header << std::endl;
    return body;
  }

  body = str_header.substr(pos + 4);
  return body;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//return HTTP method as a string (e.g GET, POST)
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string http_get_method(const std::string& str_header)
{
  std::string method;
  size_t pos = str_header.find(" ");
  if (pos == std::string::npos)
  {
    std::cout << "HTTP bad format" << std::endl;
    std::cout << str_header << std::endl;
    return method;
  }
  method = str_header.substr(0, pos);
  return method;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//escape_space
//in the query part, spaces may be encoded to either "+" (for backwards compatibility:
//do not try to search for it in the URI standard) or "%20" while the "+" character 
//(as a result of this ambiguity) has to be escaped to "%2B".
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string escape_space(const std::string& str)
{
  std::string str_out;
  size_t pos = str.find(' ');
  if (pos == std::string::npos)
  {
    str_out = str;
  }
  else
  {
    std::string s;
    for (size_t idx = 0; idx < str.size(); idx++)
    {
      if (str.at(idx) == ' ')
      {
        s += "%20";
      }
      else
      {
        s += str.at(idx);
      }
    }
    str_out = s;
  }
  return str_out;
}

