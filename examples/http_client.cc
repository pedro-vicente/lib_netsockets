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
  std::cout << "-s SERVER: fully qualified web server name (default 127.0.0.1)" << std::endl;
  std::cout << "-p PORT: server port (default 80)" << std::endl;
  std::cout << "-t 'HTTP_REQUEST', string enquoted" << std::endl;
  std::cout << "-v: verbose, output of retrieved file is printed" << std::endl;
  std::cout << "-h: help, exit" << std::endl;
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//main
//HTTP client 
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

  if (host_name.empty() || host_name.empty())
  {
    usage();
  }

  http_t client(host_name.c_str(), port);

  //open connection
  if (client.open() < 0)
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
  if (client.read_all_get_close("response.txt", verbose) < 0)
  {
    return -1;
  }

  std::ifstream ifs("response.txt", std::ios::binary);
  std::stringstream buf;
  buf << ifs.rdbuf();
  std::cout << buf.str();
  std::string str_body = http_extract_body(buf.str());
  std::ofstream ofs("response.json", std::ios::out | std::ios::binary);
  ofs.write(str_body.c_str(), str_body.size());
  ofs.close();

  client.close_socket();
  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
//mapzen_request
///////////////////////////////////////////////////////////////////////////////////////

std::string mapzen_request()
{
  const char *host_name = "search.mapzen.com";
  std::string header;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //request
  //"http://search.mapzen.com/v1/search?api_key=mapzen-hdJZGhf&text=YMCA"
  //By specifying a focus.point, nearby places will be scored higher depending on how close they are 
  //to the focus.point so that places with higher scores will appear higher in the results list. 
  //The effect of this scoring boost diminishes to zero after 100 kilometers away from the focus.point
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::string str_search = "Ella's Wood-fired Pizza";
  std::string str_lat = "38.9072";
  std::string str_lon = "-77.0369";
  str_search = escape_space(str_search);

  header += "GET /v1/search?api_key=mapzen-hdJZGhf&text=";
  header += str_search;
  header += "&size=2";
  header += "&layers=venue";
  header += "&boundary.circle.lat=";
  header += str_lat;
  header += "&boundary.circle.lon=";
  header += str_lon;
  header += "&boundary.circle.radius=30";
  header += "&focus.point.lat=";
  header += str_lat;
  header += "&focus.point.lon=";
  header += str_lon;
  header += " HTTP/1.1\r\n";
  header += "Host: ";
  header += host_name;
  header += "\r\n";
  header += "Accept: application/json\r\n";
  header += "Connection: close";
  header += "\r\n";
  header += "\r\n";
  return header;
}
