#include <string>
#include <string.h>
#include <vector>
#include <iostream>
#include <fstream>
#include "socket.hh"
#include "sqlite3.h"
#include "gason.h"
#include "sql_message.hh"

int get_response(socket_t &socket, std::string &response);
int get_http_headers(socket_t &socket, std::string &header);

/////////////////////////////////////////////////////////////////////////////////////////////////////
//usage
/////////////////////////////////////////////////////////////////////////////////////////////////////

void usage()
{
  std::cout << "-h: help, exit" << std::endl;
  std::cout << "-o PORT: port (default 3000)" << std::endl;
  std::cout << "-c: create table places" << std::endl;
  std::cout << "-t: create table items" << std::endl;
  std::cout << "-p PLACE: insert place 'PLACE'" << std::endl;
  std::cout << "-i 'ITEM': insert item 'ITEM'" << std::endl;
  std::cout << "-g: get rows from table 'places'" << std::endl;
  std::cout << "-f: get rows from table 'items'" << std::endl;
  std::cout << "-a: create table,insert place,insert item" << std::endl;
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//main
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  const char *buf_server = "127.0.0.1";
  unsigned short port = 3000;
  sql_action_t sql_action = sql_action_t::sql_none;
  std::string item;
  std::string place;

  if (argc == 1)
  {
    sql_action = sql_action_t::sql_all;
  }

  for (int idx = 1; idx < argc && argv[idx][0] == '-'; idx++)
  {
    switch (argv[idx][1])
    {
    case 'h':
      usage();
      break;
    case 'o':
      port = atoi(argv[idx + 1]);
      idx++;
      break;
    case 'c':
      sql_action = sql_action_t::sql_create_table;
      break;
    case 't':
      sql_action = sql_action_t::sql_create_table_items;
      break;
    case 'p':
      sql_action = sql_action_t::sql_insert_place;
      place = argv[idx + 1];
      idx++;
      break;
    case 'i':
      sql_action = sql_action_t::sql_insert_item;
      item = argv[idx + 1];
      idx++;
      break;
    case 'g':
      sql_action = sql_action_t::sql_get_rows_places;
      break;
    case 'f':
      sql_action = sql_action_t::sql_get_rows_items;
      break;
    case 'a':
      sql_action = sql_action_t::sql_all;
      break;
    }
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //client
  /////////////////////////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER
  Sleep(500);
#endif
  tcp_client_t client("127.0.0.1", 3000);
  if (client.open() < 0)
  {
    std::string  str = "connect error to: ";
    str += buf_server;
    std::cout << str << std::endl;
    return 1;
  }
  std::cout << "client: connected to: " << buf_server << ":" << port << std::endl;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //make sql and JSON (an array of strings)
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::string json = "[";
  sql_t sql;
  switch (sql_action)
  {
  case sql_action_t::sql_none:
    usage();
    return 0;
  case sql_action_t::sql_create_table:
    json += "\"";
    json += sql.create_table_places();
    json += "\"";
    break;
  case sql_action_t::sql_create_table_items:
    json += "\"";
    json += sql.create_table_items();
    json += "\"";
    break;
  case sql_action_t::sql_insert_place:
    assert(place.length());
    json += "\"";
    json += sql.insert_place(place.c_str());
    json += "\"";
    break;
  case sql_action_t::sql_insert_item:
    assert(item.length());
    json += "\"";
    json += sql.insert_item(item.c_str(), "home");
    json += "\"";
    break;
  case sql_action_t::sql_get_rows_places:
    json += "\"";
    json += sql.select_places(NULL);
    json += "\"";
    break;
  case sql_action_t::sql_get_rows_items:
    json += "\"";
    json += sql.select_items(NULL);
    json += "\"";
    break;
  case sql_action_t::sql_all:
    json += "\"";
    json += sql.create_table_places();
    json += "\"";
    json += ",";
    json += "\"";
    json += sql.create_table_items();
    json += "\"";
    json += ",";
    json += "\"";
    json += sql.insert_place("home");
    json += "\"";
    json += ",";
    json += "\"";
    json += sql.insert_item("it1", "home");
    json += "\"";
    json += ",";
    json += "\"";
    json += sql.select_places("home");
    json += "\"";
    json += ",";
    json += "\"";
    json += sql.select_items("home");
    json += "\"";
    break;
  }
  json += "]";

  std::ofstream ofs("request.json");
  ofs << json.c_str();
  ofs.close();

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //construct request message using sql in body
  //separate between GET and POST methods
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  char buf_request[1024];

  if (sql_action == sql_action_t::sql_get_rows_items)
  {
    sprintf(buf_request, "GET /items HTTP/1.1\r\n\r\n");
  }
  else if (sql_action == sql_action_t::sql_get_rows_places)
  {
    sprintf(buf_request, "GET /places HTTP/1.1\r\n\r\n");
  }
  else
  {
    sprintf(buf_request, "POST / HTTP/1.1\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n%s",
      strlen(json.c_str()), json.c_str());
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //send
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  if (client.write_all(buf_request, strlen(buf_request)) < 0)
  {

  }

  std::cout << "client sent: \n" << buf_request << std::endl;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //read response if there is one
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
