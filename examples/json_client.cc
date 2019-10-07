#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "socket.hh"
#include "json_message.hh"

unsigned short port = 2001;

///////////////////////////////////////////////////////////////////////////////////////
//main
//TCP client that writes and reads JSON messages 
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  char server[255]; // server host name or IP

  strcpy(server, "127.0.0.1");

  for (int i = 1; i < argc && argv[i][0] == '-'; i++)
  {
    switch (argv[i][1])
    {
    case 's':
      strcpy(server, argv[i + 1]);
      i++;
      break;
    case 'p':
      port = atoi(argv[i + 1]);
      i++;
      break;
    }
  }

  //make JSON
  char buf_json[257] = "{\"start_year\":2017}";
  tcp_client_t client(server, port);
  std::cout << "client connecting to: " << server << ":" << port << " <" << client.m_sockfd << "> " << std::endl;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //create socket and open connection
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  if (client.connect() < 0)
  {
  }
  std::cout << "client connected to: " << server << ":" << port << " <" << client.m_sockfd << "> " << std::endl;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //write request
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  if (write_request(client, buf_json) < 0)
  {
  }
  std::cout << "client sent: ";
  std::cout << buf_json << " " << server << ":" << port << " <" << client.m_sockfd << "> " << std::endl;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //read response
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::string str_response = read_response(client);
  std::cout << "client received: ";
  std::cout << str_response << " " << server << ":" << port << " <" << client.m_sockfd << "> " << std::endl;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //close connection
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  client.close();
  return 0;
}



