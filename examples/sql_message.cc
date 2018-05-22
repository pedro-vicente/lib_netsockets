#include <string>
#include <string.h>
#include <vector>
#include <iostream>
#include <fstream>
#include "sql_message.hh"

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
  if (NULL == place)
  {
    sql += "SELECT * FROM table_places ;";
  }
  else
  {
    sql += "SELECT * FROM table_places WHERE place_id = '";
    sql += place;
    sql += "';";
  }
  return sql.c_str();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//sql_t::select_items
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string sql_t::select_items(const char* place)
{
  std::string sql;
  if (NULL == place)
  {
    sql += "SELECT * FROM table_items ;";
  }
  else
  {
    sql += "SELECT * FROM table_items WHERE place_id = '";
    sql += place;
    sql += "';";
  }
  return sql.c_str();
}
