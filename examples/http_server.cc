#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "socket.hh"

bool verbose = true;

/////////////////////////////////////////////////////////////////////////////////////////////////////
//usage
/////////////////////////////////////////////////////////////////////////////////////////////////////

void usage()
{
  std::cout << "-p PORT: server port (default 3000)" << std::endl;
  std::cout << "-v: verbose output" << std::endl;
  std::cout << "-h: help, exit" << std::endl;
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//main
///////////////////////////////////////////////////////////////////////////////////////

int handle_client(socket_t& socket);
std::string do_plot();
std::string do_map();
std::string read_map();

int main(int argc, char *argv[])
{
  unsigned short port = 3000;

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

  tcp_server_t server(port);
  std::cout << "server listening on port " << port << std::endl;
  while (true)
  {
    socket_t socket = server.accept_client();

    // convert IP addresses from a dots-and-number string to a struct in_addr
    char *str_ip = inet_ntoa(socket.m_sockaddr_in.sin_addr);
    std::cout << prt_time() << "server accepted: " << str_ip << "," << socket.m_socket_fd << std::endl;

    if (handle_client(socket) < 0)
    {
      std::cout << "error on client handling";
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
  std::string str_header;
  char buf[4096];

  if (socket.parse_http_headers(str_header) < 0)
  {
    std::cout << "parse_http_headers error\n";
    return -1;
  }

  unsigned long long size_body = http_get_field("Content-Length: ", str_header);

  if (verbose)
  {
    std::cout << "received: Content-Length: " << size_body << std::endl;
  }

  //now get body using size of Content-Length
  if (socket.read_all(buf, (int)size_body) < 0)
  {
    std::cout << "recv error: " << strerror(errno) << std::endl;
    return -1;
  }

  std::string str_body(buf, (unsigned int)size_body);

  if (verbose)
  {
    std::cout << "received: " << str_body << std::endl;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //response
  //response is a simple HTTP 200 OK reply followed by script
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::string str("HTTP/1.1 200 OK\r\n\r\n");
  str += read_map();
  str += do_map();

  if (verbose)
  {
    std::cout << str << std::endl;
  }

  if (socket.write_all(str.c_str(), str.size()) < 0)
  {
    std::cout << "write response error\n";
    return -1;
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//do_plot
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string do_plot()
{
  std::string str;
  str += "<!doctype html><html>";
  str += "<head>";
  str += "</head>";
  str += "<body>server<div id='div_id'></div>";
  str += "<script src='https://cdn.plot.ly/plotly-latest.min.js'></script>";
  str += "<script>";
  str += "var trace1 = {";
  str += "x: [2, 3, 4, 5],";
  str += "y: [16, 5, 11, 10],";
  str += "mode: 'lines+markers'";
  str += "};";
  str += "var data = [trace1];";
  str += "var layout = {};";
  str += "Plotly.newPlot('div_id', data, layout);";
  str += "</script></body></html>";
  return str;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//read_map
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string read_map()
{
  std::ifstream ifs("index.leaflet.html");
  std::stringstream strm;
  strm << ifs.rdbuf();
  return strm.str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//do_map
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string do_map()
{
  std::ostringstream strm;
  strm
    << "<script>"
    << "var layer_base = L.tileLayer(\n"
    << "'http://cartodb-basemaps-{s}.global.ssl.fastly.net/light_all/{z}/{x}/{y}@2x.png',{\n"
    << "opacity: 1\n"
    << "});\n"
    << "var map = new L.Map('map', {\n"
    << "center: new L.LatLng(38.9072, -77.0369),\n"
    << "zoom: 13,\n"
    << "layers: [layer_base]\n"
    << "});\n"
    << "var circle = L.circle([38.9072, -77.0369], {"
    << "color: '#ff0000',"
    << "stroke: false,"
    << "radius : 500"
    << "}).addTo(map);"
    << "</script>";
  return strm.str();
}