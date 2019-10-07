#include <string>
#include <string.h>
#include <vector>
#include <iostream>
#include <fstream>
#include "socket.hh"
#include "http.hh"
#include "gason.h"
#include "sql_message.hh"

int get_response(socket_t& socket, std::string& response);

/////////////////////////////////////////////////////////////////////////////////////////////////////
//usage
//-u http://127.0.0.1/get_places
//-u http://127.0.0.1/get_items
/////////////////////////////////////////////////////////////////////////////////////////////////////

void usage()
{
  std::cout << "usage: sqlite_client -u URI <-p PORT> <-h>" << std::endl;
  std::cout << "-u URI" << std::endl;
  std::cout << "-h: help, exit" << std::endl;
  std::cout << "-o PORT: port (default 3000)" << std::endl;
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//main
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  std::string uri;
  unsigned short port = 3000;
  sql_action_t sql_action = sql_action_t::none;
  std::string item;
  std::string place;

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
    case 'u':
      uri = argv[idx + 1];
      idx++;
      break;
    }
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //client
  /////////////////////////////////////////////////////////////////////////////////////////////////////


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

  //get API token
  std::string api_token = uri.substr(end);

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //generate SQL from API
  //API:
  // /get_places
  // /get_items
  // /create_table_places
  // /create_table_items
  // /insert_place
  // /insert_item
  // /select_places
  // /select_items
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  if (api_token.compare("/get_places") == 0)
  {
    sql_action = sql_action_t::get_places;
  }
  else if (api_token.compare("/get_items") == 0)
  {
    sql_action = sql_action_t::get_items;
  }
  else if (api_token.compare("/create_table_places") == 0)
  {
    sql_action = sql_action_t::create_table_places;
  }
  else if (api_token.compare("/create_table_items") == 0)
  {
    sql_action = sql_action_t::create_table_items;
  }
  else if (api_token.find("/insert_place/") != std::string::npos)
  {
    sql_action = sql_action_t::insert_place;
    place = api_token.substr(std::string("/insert_place/").size());
  }
  else if (api_token.find("/insert_item/") != std::string::npos)
  {
    sql_action = sql_action_t::insert_item;
    item = api_token.substr(std::string("/insert_item/").size());
  }
  else if (api_token.compare("/select_places") == 0)
  {
    sql_action = sql_action_t::select_places;
  }
  else if (api_token.compare("/select_places") == 0)
  {
    sql_action = sql_action_t::insert_item;
  }
  else if (api_token.compare("/select_items") == 0)
  {
    sql_action = sql_action_t::select_items;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //make sql and JSON (an array of strings)
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::string json = "[";
  sql_t sql;
  switch (sql_action)
  {
  case sql_action_t::none:
    usage();
    return 0;
  case sql_action_t::create_table_places:
    json += "\"";
    json += sql.create_table_places();
    json += "\"";
    break;
  case sql_action_t::create_table_items:
    json += "\"";
    json += sql.create_table_items();
    json += "\"";
    break;
  case sql_action_t::insert_place:
    assert(place.length());
    json += "\"";
    json += sql.insert_place(place.c_str());
    json += "\"";
    break;
  case sql_action_t::insert_item:
    assert(item.length());
    json += "\"";
    json += sql.insert_item(item.c_str(), "home");
    json += "\"";
    break;
  case sql_action_t::select_places:
    json += "\"";
    json += sql.select_places(NULL);
    json += "\"";
    break;
  case sql_action_t::select_items:
    json += "\"";
    json += sql.select_items(NULL);
    json += "\"";
    break;
  case sql_action_t::all:
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
  //make HTTP request
  //construct request message using sql in body
  //separate between GET and POST methods
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  char buf_request[1024];

  if (sql_action == sql_action_t::get_items)
  {
    sprintf(buf_request, "GET /get_items HTTP/1.1\r\n\r\n");
  }
  else if (sql_action == sql_action_t::get_places)
  {
    sprintf(buf_request, "GET /get_places HTTP/1.1\r\n\r\n");
  }
  else
  {
    sprintf(buf_request, "POST / HTTP/1.1\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n%s",
      strlen(json.c_str()), json.c_str());
  }


#if _MSC_VER
  Sleep(500);
#endif

  tcp_client_t client(host_name.c_str(), port);
  std::cout << "client connecting to: " << host_name << ":" << port << " <" << client.m_sockfd << "> " << std::endl;

  if (client.connect() < 0)
  {
    std::string  str = "connect error to: ";
    str += host_name;
    std::cout << str << std::endl;
    return 1;
  }
  std::cout << "client: connected to: " << host_name << ":" << port << std::endl;

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

  client.close();
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//get_response
//return value
//0 nothing received
//1 something received
/////////////////////////////////////////////////////////////////////////////////////////////////////

int get_response(socket_t& socket, std::string& response)
{
  //get HTTP header
  std::string header;
  if (parse_http_headers(socket.m_sockfd, header) == 0)
  {
    return 0;
  }

  //get size
  unsigned int size_body = (unsigned int)http_get_field("Content-Length: ", header);

  //read from socket with known size
  if (size_body)
  {
    //read from socket with known size
    char* buf = new char[size_body];
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


