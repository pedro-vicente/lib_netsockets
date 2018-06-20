#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "socket.hh"

int get_response(socket_t &socket, std::string &response);
int get_http_headers(socket_t &socket, std::string &header);
const std::string ftams_place("/FTAMSDeviceService/DeviceService.svc/Reports/GetMessages/");

/////////////////////////////////////////////////////////////////////////////////////////////////////
//usage
//File Transfer Access Management Services
//-u http://127.0.0.1/FTAMSDeviceService/DeviceService.svc/Reports/GetMessages/3
/////////////////////////////////////////////////////////////////////////////////////////////////////

void usage()
{
  std::cout << "usage: ftams_client -u URI <-p PORT> <-h>" << std::endl;
  std::cout << "-u URI" << std::endl;
  std::cout << "-p PORT: server port (default 4000)" << std::endl;
  std::cout << "-v: verbose, output of retrieved file is printed" << std::endl;
  std::cout << "-h: help, exit" << std::endl;
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//main
//FTAMS client 
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  std::string uri;
  unsigned short port = 4000;
  bool verbose = false;

  for (int i = 1; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      switch (argv[i][1])
      {
      case 'h':
        usage();
        break;
      case 'v':
        verbose = true;
        break;
      case 'u':
        uri = argv[i + 1];
        i++;
        break;
      case 'p':
        port = atoi(argv[i + 1]);
        i++;
        break;
      default:
        usage();
      }
    }
    else
    {
      usage();
    }
  }

  if (uri.empty())
  {
    usage();
  }

  //validate
  std::size_t ftams_place_pos = uri.find(ftams_place, 0);
  if (ftams_place_pos == std::string::npos)
  {
    usage();
  }

  //parse URI 
  std::size_t start = uri.find("://", 0);
  if (start == std::string::npos)
  {
    usage();
  }
  start += 3;
  std::size_t end = uri.find("/", start + 1);
  std::string host_name = uri.substr(start, end - start);
  std::cout << "host: " << host_name << std::endl;

#if _MSC_VER
  Sleep(500);
#endif

  tcp_client_t client(host_name.c_str(), port);
  std::cout << "client connecting to: " << host_name << ":" << port << " <" << client.m_socket_fd << "> " << std::endl;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //create socket and open connection
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  if (client.open() < 0)
  {
    return 1;
  }
  std::cout << "client connected to: " << host_name << ":" << port << " <" << client.m_socket_fd << "> " << std::endl;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //make HTTP request
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  //get API token
  std::string api_token = uri.substr(ftams_place_pos);
  std::string http_request = "GET ";
  http_request += api_token;
  http_request += " HTTP/1.1\r\n\r\n";

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //send
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  if (client.write_all(http_request.c_str(), http_request.size()) < 0)
  {

  }

  std::cout << "client sent: \n" << http_request << std::endl;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //read response 
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::string response;
  if (get_response(client, response) > 0)
  {
    std::cout << "client received:\n";
    std::cout << response << std::endl;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //close connection
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  client.close_socket();
  std::cout << "client closed to: " << host_name << ":" << port << " <" << client.m_socket_fd << "> " << std::endl;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//get_response
//return value
//0 nothing received
//1 something received
/////////////////////////////////////////////////////////////////////////////////////////////////////

int get_response(socket_t &socket, std::string &response)
{
  //get HTTP header
  std::string header;
  if (get_http_headers(socket, header) == 0)
  {
    return 0;
  }

  //get size
  unsigned int size_body = (unsigned int)http_get_field("Content-Length: ", header);

  //read from socket with known size
  if (size_body)
  {
    //read from socket with known size
    char *buf = new char[size_body];
    if (socket.read_all(buf, size_body) < 0)
    {
      std::cout << "recv error: " << strerror(errno) << std::endl;
    }
    std::string str_json(buf, size_body);
    response = str_json;
    delete[] buf;
    return 1;
  }

  return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//parse_http_headers
//return value
//0 nothing received
//1 something received
/////////////////////////////////////////////////////////////////////////////////////////////////////

int get_http_headers(socket_t &socket, std::string &header)
{
  int recv_size; // size in bytes received or -1 on error 
  const int size_buf = 4096;
  char buf[size_buf];

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //MSG_PEEK
  //Peeks at an incoming message.The data is treated as unread and the next recv() or similar 
  //function shall still return this data.
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  if ((recv_size = recv(socket.m_socket_fd, buf, size_buf, MSG_PEEK)) == -1)
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
  if ((recv_size = recv(socket.m_socket_fd, buf, header_len, 0)) == -1)
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
