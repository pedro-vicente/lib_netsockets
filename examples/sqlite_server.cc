#include <string>
#include <string.h>
#include <vector>
#include <iostream>
#include <fstream>
#include "socket.hh"
#include "http.hh"
#include "sqlite3.h"
#include "gason.h"
#include "sql_message.hh"

int handle_sql(const std::string& db_path, const std::string& sql, std::string &json, std::string& sql_errmsg);
int handle_client(socket_t& socket, const std::string& db_path);
int send_http_response(socket_t& socket, std::string& msg);

/////////////////////////////////////////////////////////////////////////////////////////////////////
//usage
/////////////////////////////////////////////////////////////////////////////////////////////////////

void usage()
{
  std::cout << "-h help, exit" << std::endl;
  std::cout << "-o PORT: port (default 3000)" << std::endl;
  std::cout << "-d DATABASE: full path of SQLite database file" << std::endl;
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//main
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  unsigned short port = 3000;
  std::string db_path;

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
    case 'd':
      db_path = argv[idx + 1];
      idx++;
      break;
    }
  }

  if (db_path.empty())
  {
    std::cout << "database path missing" << std::endl;
    return 1;
  }

  sqlite3 *db;
  std::cout << "server: database: " << db_path.c_str() << std::endl;
  if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK)
  {
    std::string sql_errmsg = db_path;
    sql_errmsg += sqlite3_errmsg(db);
    std::cout << sql_errmsg << std::endl;
  }
  sqlite3_close(db);

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //server
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  tcp_server_t server(port);

  std::cout << "server: listening on port " << port << std::endl;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //server loop
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  while (true)
  {
    socket_t socket = server.accept();

    // convert IP addresses from a dots-and-number string to a struct in_addr
    char *str_ip = inet_ntoa(socket.m_sockaddr_in.sin_addr);
    std::cout << prt_time() << "server: accepted: " << str_ip << "," << socket.m_sockfd << std::endl;

    if (handle_client(socket, db_path) < 0)
    {

    }
    socket.close();
  }
  server.close();
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//handle_client
/////////////////////////////////////////////////////////////////////////////////////////////////////

int handle_client(socket_t& socket, const std::string& db_path)
{
  std::string header;
  std::string json; //response
  char buf[4096];

  if (parse_http_headers(socket.m_sockfd, header) < 0)
  {
    std::cout << "parse_http_headers error\n";
    return -1;
  }

  std::string method = http_get_method(header);

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //GET method
  //send back JSON response
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  if (method.compare("GET") == 0)
  {
    size_t start = header.find("/");
    size_t end = header.find(" ", start);
    std::string action = header.substr(start + 1, end - start - 1);
    std::cout << "REST method: " << action << "\n";
    std::string sql;
    std::string msg;
    if (action.compare("get_items") == 0)
    {
      sql = "SELECT * FROM table_items ;";
    }
    else if (action.compare("get_places") == 0)
    {
      sql = "SELECT * FROM table_places ;";
    }
    if (handle_sql(db_path, sql, json, msg) == SQLITE_ERROR)
    {
      send_http_response(socket, msg);
      return -1;
    }
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //POST method
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  else if (method.compare("POST") == 0)
  {
    unsigned long long size_body = http_get_field("Content-Length: ", header);
    std::cout << "received: Content-Length: " << size_body << std::endl;
    if (size_body == 0)
    {
      //send response to client
      std::string msg("Invalid request");
      send_http_response(socket, msg);
      return 0;
    }

    //now get body using size of Content-Length
    if (socket.read_all(buf, (int)size_body) < 0)
    {
      std::cout << "recv error: " << strerror(errno) << std::endl;
      return -1;
    }

    char *endptr;
    JsonValue value;
    JsonAllocator allocator;
    int status = jsonParse(buf, &endptr, &value, allocator);
    if (status != JSON_OK)
    {
      std::cout << "invalid JSON format for " << buf << std::endl;
      return -1;
    }

    std::cout << std::endl << "Parsing SQL in JSON..." << std::endl << std::endl;

    //JSON is an array of strings, each string is a SQL statement
    std::vector<std::string> vec_sql;
    for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
    {
      vec_sql.push_back(node->value.toString());
    }

    std::cout << std::endl << "Executing SQL to database..." << std::endl << std::endl;

    size_t nbr_sql = vec_sql.size();
    for (size_t idx = 0; idx < nbr_sql; idx++)
    {
      std::string msg;
      if (handle_sql(db_path, vec_sql.at(idx), json, msg) == SQLITE_ERROR)
      {
        //send response to client
        send_http_response(socket, msg);
        return -1;
      }
    }
  }
  else
  {
    std::cout << "invalid HTTP header method: " << method.c_str() << "\n";
    return -1;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //response
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::string response("HTTP/1.1 200 OK\r\n");
  response += "Content-Type: application/json\r\n";
  response += "Content-Length: ";
  response += std::to_string(json.size());
  response += "\r\n";
  response += "\r\n"; //terminate HTTP headers
  response += json;
  if (socket.write_all(response.c_str(), response.size()) < 0)
  {
    std::cout << "write response error\n";
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//handle_sql
/////////////////////////////////////////////////////////////////////////////////////////////////////

int handle_sql(const std::string& db_path, const std::string& sql, std::string &json, std::string& sql_errmsg)
{
  sqlite3 *db;
  sqlite3_stmt *stmt;
  int rc;

  std::cout << "SQL:" << std::endl;
  std::cout << sql.c_str() << std::endl;

  if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK)
  {
    sql_errmsg = db_path;
    sql_errmsg += sqlite3_errmsg(db);
    std::cout << sql_errmsg << std::endl;
    return SQLITE_ERROR;
  }

  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK)
  {
    sql_errmsg = sqlite3_errmsg(db);
    std::cout << sql_errmsg << std::endl;;
    return SQLITE_ERROR;
  }

  std::vector<std::string> vec;
  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
  {
    const unsigned char *field_0 = sqlite3_column_text(stmt, 0);
    const unsigned char *field_1 = sqlite3_column_text(stmt, 1);
    int field_2 = sqlite3_column_int(stmt, 2);

    //unfortunately, sqlite3_column_int returns integer 0 for error

    std::string str((char*)field_0);
    str += " ";
    str += (char*)field_1;
    vec.push_back(str);
    std::cout << str.c_str() << std::endl;
  }

  //make JSON reply (an array of strings)
  json = "[";
  for (int idx = 0; idx < vec.size(); idx++)
  {
    json += "\"";
    json += vec.at(idx);
    json += "\"";
    if (idx < vec.size() - 1)
    {
      json += ",";
    }
  }
  json += "]";

  std::ofstream ofs("response.json");
  ofs << json.c_str();
  ofs.close();

  if (sqlite3_finalize(stmt) != SQLITE_OK)
  {
    sql_errmsg = sqlite3_errmsg(db);
    std::cout << sql_errmsg << std::endl;;
    sqlite3_close(db);
    return SQLITE_ERROR;
  }
  sqlite3_close(db);
  return SQLITE_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//send_http_response 
//send a HTTP message to client
/////////////////////////////////////////////////////////////////////////////////////////////////////

int send_http_response(socket_t& socket, std::string& msg)
{
  //send response to client
  std::string response("HTTP/1.1 200 OK\r\n");
  response += "Content-Length: ";
  response += std::to_string(msg.size());
  response += "\r\n";
  response += "\r\n"; //terminate HTTP
  response += msg;
  if (socket.write_all(response.c_str(), response.size()) < 0)
  {
    std::cout << "write response error\n";
  }
  return 0;
}