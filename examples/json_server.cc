#define _CRT_NONSTDC_NO_DEPRECATE
#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <ctime>
#include "socket.hh"
#include "gason.h"
#include "json_message.hh"

unsigned short port = 2001;

///////////////////////////////////////////////////////////////////////////////////////
//main
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  std::string argv_str(argv[0]);
  std::string path = argv_str.substr(0, argv_str.find_last_of("/"));

  for (int i = 1; i < argc && argv[i][0] == '-'; i++)
  {
    switch (argv[i][1])
    {
    case 'p':
      port = atoi(argv[i + 1]);
      i++;
      break;
    case 'd':
      set_daemon(path.c_str());
      break;
    }
  }

  tcp_server_t server(port);
  std::cout << "server listening on port " << port << std::endl;
  while (true)
  {
    socket_t socket = server.accept();

    // convert IP addresses from a dots-and-number string to a struct in_addr and back
    char* str_ip = inet_ntoa(socket.m_sockaddr_in.sin_addr);
    std::cout << prt_time() << "server accepted: " << str_ip << " <" << socket.m_sockfd << "> " << std::endl;

    std::string str_request = read_response(socket);
    std::cout << prt_time() << "server received: " << str_request << std::endl;
    char* buf_request = strdup(str_request.c_str());

    //get dates
    char *endptr;
    JsonValue value;
    JsonAllocator allocator;
    int status = jsonParse(buf_request, &endptr, &value, allocator);
    if (status != JSON_OK)
    {
      std::cout << "invalid JSON format for " << buf_request << std::endl;
      return -1;
    }

    int year;
    for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
    {
      std::cout << node->key;
      year = (int)node->value.toNumber();
    }

    std::string json = "{\"next_year\":";
    json += std::to_string(year + 1);
    json += "}";

    if (write_request(socket, json.c_str()) < 0)
    {
    }

    std::cout << prt_time() << "server sent: " << json.c_str() << std::endl;
    socket.close();
    free(buf_request);
  }
  server.close();
  return 0;
}


