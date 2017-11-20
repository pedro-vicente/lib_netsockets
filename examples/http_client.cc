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
  std::cout << "usage: http_client -s SERVER <-p PORT> <-g> <-f /FILE> <-v> <-h>" << std::endl;
  std::cout << "-s SERVER: fully qualified web server name (default 127.0.0.1)" << std::endl;
  std::cout << "-p PORT: server port (default 80)" << std::endl;
  std::cout << "-g Use GET of -f FILE, otherwise POST is used" << std::endl;
  std::cout << "-f FILE: file located at web server root; file path name must start with '/' (default index.html)" << std::endl;
  std::cout << "-v: verbose, output of retrieved file is printed" << std::endl;
  std::cout << "-w : connect to a web service " << std::endl;
  std::cout << "-h: help, exit" << std::endl;
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//main
//HTTP client 
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  const char *host_name = "127.0.0.1"; // server name 
  const char *path_name = "/"; // name of file to retrieve
  unsigned short port = 80;
  bool verbose = false;
  bool get = true;
  bool mapzen = true;

  if (mapzen)
  {
    host_name = "search.mapzen.com";
  }

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
      case 'g':
        get = true;
        break;
      case 'w':
        mapzen = true;
        host_name = "search.mapzen.com";
        break;
      case 's':
        host_name = argv[i + 1];
        i++;
        break;
      case 'f':
        path_name = argv[i + 1];
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

  http_t client(host_name, port);

  //open connection
  if (client.open() < 0)
  {

  }

  std::cout << "client connected to: " << host_name << ":" << port << std::endl;

  if (get)
  {
    std::string header;

    // -s search.mapzen.com -p 80 -w
    if (mapzen)
    {
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

      std::cout << header;

      //send request, using built in tcp_client_t socket
      if (client.write_all(header.c_str(), header.size()) < 0)
      {
        return -1;
      }

      //we sent a close() server request, so we can use the read_all function
      //that checks for recv() return value of zero (connection closed)
      if (client.read_all_get_close("mapzen.response.txt", verbose) < 0)
      {
        return -1;
      }

      std::ifstream ifs("mapzen.response.txt", std::ios::binary);
      std::stringstream buf;
      buf << ifs.rdbuf();
      std::cout << buf.str();
      std::string str_body = http_extract_body(buf.str());
      std::ofstream ofs("mapzen.response.json", std::ios::out | std::ios::binary);
      ofs.write(str_body.c_str(), str_body.size());
      ofs.close();
    }
    else
    {
      client.get(path_name, verbose);
    }

  }
  else
  {
    std::string str_body("start_year=2016&end_year=2017");
    client.post(str_body);
  }
  client.close_socket();
  return 0;
}

