#include <string>
#include <string.h>
#include <vector>
#include <iostream>
#include <fstream>
#include "socket.hh"
#include "sqlite3.h"
#include "gason.h"
#include "sql_message.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//usage
/////////////////////////////////////////////////////////////////////////////////////////////////////

void usage()
{
  std::cout << "-h: help, exit" << std::endl;
  std::cout << "-o PORT: port (default 3000)" << std::endl;
  std::cout << "-c: create table places" << std::endl;
  std::cout << "-t: create table items" << std::endl;
  std::cout << "-p: insert place" << std::endl;
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
    json += "\"";
    json += sql.insert_place("home");
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

  //construct request message using sql in body
  char buf_request[1024];
  sprintf(buf_request, "POST / HTTP/1.1\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n%s",
    strlen(json.c_str()), json.c_str());

  if (client.write_all(buf_request, strlen(buf_request)) < 0)
  {

  }

  std::cout << "client sent: \n" << buf_request << std::endl;

  client.close_socket();


  return 0;
}



