#include <string>
#include <string.h>
#include <vector>
#include <iostream>
//local
#include "socket.hh"
//external
#include "sqlite3.h"
#include "gason.h"

//type of sql action to do
enum class sql_action_t
{
  sql_none,
  sql_create_table,
  sql_create_table_items,
  sql_insert_place,
  sql_insert_item,
  sql_get_rows_places,
  sql_get_rows_items,
  sql_all
};

int handle_sql(const std::string& sql);

/////////////////////////////////////////////////////////////////////////////////////////////////////
//sql_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

class sql_t
{
public:
  sql_t() {};
  std::string create_table_places();
  std::string create_table_items();
  std::string insert_place(const char* place);
  std::string insert_item(const char* item, const char* place);
  std::string select_places(const char* place);
  std::string select_items(const char* place);
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
//usage
/////////////////////////////////////////////////////////////////////////////////////////////////////

void usage()
{
  std::cout << "-h: help, exit" << std::endl;
  std::cout << "-o PORT: server port (default 3000)" << std::endl;
  std::cout << "-c: create table places" << std::endl;
  std::cout << "-t: create table items" << std::endl;
  std::cout << "-p: insert place" << std::endl;
  std::cout << "-i: insert item" << std::endl;
  std::cout << "-g: get rows from table 'places'" << std::endl;
  std::cout << "-f: get rows from table 'items'" << std::endl;
  std::cout << "-a: create table,insert place,insert item" << std::endl;
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//main
///////////////////////////////////////////////////////////////////////////////////////

int handle_client(socket_t& socket);

int main(int argc, char *argv[])
{
  const char *buf_server = "127.0.0.1";
  unsigned short port = 3000;
  sql_action_t sql_action = sql_action_t::sql_none;

  if (argc == 1)
  {
    sql_action = sql_action_t::sql_all;
  }

  for (int i = 1; i < argc && argv[i][0] == '-'; i++)
  {
    switch (argv[i][1])
    {
    case 'h':
      usage();
      break;
    case 'o':
      port = atoi(argv[i + 1]);
      i++;
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
  //server
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  tcp_server_t server(port);
  std::cout << "server: listening on port " << port << std::endl;

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //client
  /////////////////////////////////////////////////////////////////////////////////////////////////////

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
    json += "\"";
    json += sql.insert_item("it1", "home");
    json += "\"";
    break;
  case sql_action_t::sql_get_rows_places:
    json += "\"";
    json += sql.select_places("home");
    json += "\"";
    break;
  case sql_action_t::sql_get_rows_items:
    json += "\"";
    json += sql.select_items("home");
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

  //construct request message using sql in body
  char buf_request[1024];
  sprintf(buf_request, "POST / HTTP/1.1\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n%s",
    strlen(json.c_str()), json.c_str());

  if (client.write_all(buf_request, strlen(buf_request)) < 0)
  {

  }

  std::cout << "client sent: \n" << buf_request << std::endl;

  client.close_socket();

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //server loop
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  while (true)
  {
    socket_t socket = server.accept_client();

    // convert IP addresses from a dots-and-number string to a struct in_addr
    char *str_ip = inet_ntoa(socket.m_sockaddr_in.sin_addr);
    std::cout << prt_time() << "server: accepted: " << str_ip << "," << socket.m_socket_fd << std::endl;

    if (handle_client(socket) < 0)
    {

    }
    socket.close_socket();
    exit(0);
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

  unsigned long long size_body = http_extract_field("Content-Length: ", str_header);
  std::cout << "received: Content-Length: " << size_body << std::endl;

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

  std::vector<std::string> vec_sql;
  for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
  {
    std::cout << node->value.toString() << std::endl;
    vec_sql.push_back(node->value.toString());

  }

  std::cout << std::endl << "Executing SQL to database..." << std::endl << std::endl;

  size_t nbr_sql = vec_sql.size();
  for (size_t idx = 0; idx < nbr_sql; idx++)
  {
    handle_sql(vec_sql.at(idx));
  }

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//handle_sql
/////////////////////////////////////////////////////////////////////////////////////////////////////

int handle_sql(const std::string& sql)
{
  sqlite3 *db;
  sqlite3_stmt *stmt;
  int rc;

  std::cout << "SQL:" << std::endl;
  std::cout << sql.c_str() << std::endl;

  if (sqlite3_open("test.sqlite", &db) != SQLITE_OK)
  {
    std::cout << sqlite3_errmsg(db);
    return SQLITE_ERROR;
  }

  if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK)
  {
    std::cout << sqlite3_errmsg(db);
    sqlite3_close(db);
    return SQLITE_ERROR;
  }

  while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
  {
    const unsigned char *id = sqlite3_column_text(stmt, 0);
    const unsigned char *address = sqlite3_column_text(stmt, 1);
    int rank = sqlite3_column_int(stmt, 2);

    std::cout << "id: " << id << std::endl;
  }

  if (sqlite3_finalize(stmt) != SQLITE_OK)
  {
    std::cout << sqlite3_errmsg(db) << std::endl;;
    sqlite3_close(db);
    return SQLITE_ERROR;
  }
  sqlite3_close(db);
  return SQLITE_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//sql_t::create_table_places
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string sql_t::create_table_places()
{
  std::string sql;
  sql += "CREATE TABLE IF NOT EXISTS table_places(";
  sql += "place_id TEXT PRIMARY KEY NOT NULL,"; //0, name
  sql += "address CHAR(50) NOT NULL,"; //1
  sql += "rank INTEGER NOT NULL);"; //2
  return sql.c_str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//sql_t::create_table_items
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string sql_t::create_table_items()
{
  std::string sql;
  sql += "CREATE TABLE IF NOT EXISTS table_items(";
  sql += "path TEXT PRIMARY KEY NOT NULL,"; //0
  sql += "place_id TEXT NOT NULL, FOREIGN KEY(place_id) REFERENCES table_places(place_id)"; //1
  sql += ");";
  return sql.c_str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//sql_t::insert_place
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string sql_t::insert_place(const char* place)
{
  std::string sql;
  sql += "INSERT INTO table_places ";
  sql += "VALUES('";
  sql += place;
  sql += "', '102 E. Green St. Urbana IL 61801', 1);";
  return sql.c_str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//sql_t::insert_item
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string sql_t::insert_item(const char* item, const char* place)
{
  std::string sql;
  sql += "INSERT INTO table_items ";
  sql += "VALUES('";
  sql += item;
  sql += "','";
  sql += place;
  sql += "');";
  return sql.c_str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//sql_t::select_places
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string sql_t::select_places(const char* place)
{
  std::string sql;
  sql += "SELECT * FROM table_places WHERE place_id = '";
  sql += place;
  sql += "';";
  return sql.c_str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//sql_t::select_items
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string sql_t::select_items(const char* place)
{
  std::string sql;
  sql += "SELECT * FROM table_items WHERE place_id = '";
  sql += place;
  sql += "';";
  return sql.c_str();
}
