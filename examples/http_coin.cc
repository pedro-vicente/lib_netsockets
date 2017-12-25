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
  std::cout << "usage: http_coin -k KEY -s SECRET <-v> <-h>" << std::endl;
  std::cout << "-k: key" << std::endl;
  std::cout << "-s: secret" << std::endl;
  std::cout << "-v: verbose" << std::endl;
  std::cout << "-h: help, exit" << std::endl;
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//main
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  const char *host_name = "www.cryptocurrencychart.com"; // server name 
  unsigned short port = 80;
  bool verbose = false;
  std::string key;
  std::string secret;

  for (int i = 1; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      switch (argv[i][1])
      {
      case 'h':
        usage();
        break;
      case 'k':
        key = argv[i + 1];
        i++;
        break;
      case 's':
        secret = argv[i + 1];
        i++;
        break;
      case 'v':
        verbose = true;
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

  if (key.empty() || secret.empty())
  {
    usage();
  }

  http_t client(host_name, port);

  //open connection
  if (client.open() < 0)
  {
    return -1;
  }

  std::cout << "client connected to: " << host_name << ":" << port << std::endl;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //request
  //"http://www.cryptocurrencychart.com/api/coin/history/363/2017-01-01/2017-01-02/marketCap/usd"
  //'Key: yourKey', 'Secret: yourSecret'
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::string header;
  header += "GET /api/coin/history/363/2017-01-01/2017-01-02/marketCap/usd";
  header += " HTTP/1.1\r\n";
  header += "Host: ";
  header += host_name;
  header += "\r\n";
  header += "Accept: application/json\r\n";
  header += "Key: ";
  header += key;
  header += "\r\n";
  header += "Secret: ";
  header += secret;
  header += "\r\n";
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
  if (client.read_all_get_close("coin.response.txt", verbose) < 0)
  {
    return -1;
  }

  std::ifstream ifs("coin.response.txt", std::ios::binary);
  std::stringstream buf;
  buf << ifs.rdbuf();
  std::cout << buf.str();
  std::string str_body = http_extract_body(buf.str());
  std::ofstream ofs("coin.response.json", std::ios::out | std::ios::binary);
  ofs.write(str_body.c_str(), str_body.size());
  ofs.close();


  client.close_socket();
  return 0;
}

