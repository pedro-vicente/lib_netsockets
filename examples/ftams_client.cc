#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <string.h>
#include "socket.hh"

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
  //make and HTTP request
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
  //close connection
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  client.close_socket();
  std::cout << "client closed to: " << host_name << ":" << port << " <" << client.m_socket_fd << "> " << std::endl;
  return 0;
}


