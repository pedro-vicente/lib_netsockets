#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <string.h>
#include "http.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//usage
/////////////////////////////////////////////////////////////////////////////////////////////////////

void usage()
{
  std::cout << "usage: http_client -s SERVER -t 'HTTP_REQUEST' <-p PORT> <-v> <-h>" << std::endl;
  std::cout << "-s SERVER: fully qualified web server name" << std::endl;
  std::cout << "-p PORT: server port (default 80)" << std::endl;
  std::cout << "-t 'HTTP_REQUEST', string enquoted" << std::endl;
  std::cout << "-v: verbose, output of retrieved file is printed" << std::endl;
  std::cout << "-h: help, exit" << std::endl;
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//main
//HTTP client 
//-s 127.0.0.1 -p 3000
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  std::string host_name;
  std::string http_request;
  unsigned short port = 80;
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
      case 's':
        host_name = argv[i + 1];
        i++;
        break;
      case 't':
        http_request = argv[i + 1];
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

  if (host_name.empty())
  {
    usage();
  }

  http_client_t client(host_name.c_str(), port);

  //open connection
  if (client.connect() < 0)
  {

  }

  std::cout << "client connected to: " << host_name << ":" << port << std::endl;

  std::string header = http_request;
  header += " HTTP/1.1\r\n";
  header += "Host: ";
  header += host_name;
  header += "\r\n";
  header += "Accept: application/json\r\n";
  header += "Connection: close";
  header += "\r\n";
  header += "\r\n";

  std::cout << header;

  //send request, using built in tcp_client_t socket
  if (client.write_all(header.c_str(), header.size()) < 0)
  {
    return -1;
  }

  //we sent a close() server request, so we can use the read_all function
  //that checks for recv() return value of zero (connection closed)
  char buf_[4096];
  int size_read;
  if ((size_read = client.read_all(buf_, sizeof(buf_))) < 0)
  {
    return -1;
  }

  std::string buf(buf_, size_read);
  std::cout << buf.c_str();
  std::string str_body = http_get_body(buf.c_str());
  std::ofstream ofs("response.json", std::ios::out | std::ios::binary);
  ofs.write(str_body.c_str(), str_body.size());
  ofs.close();

  client.close();
  return 0;
}


