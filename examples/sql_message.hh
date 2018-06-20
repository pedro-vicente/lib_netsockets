#ifndef LIB_NETSOCKET_SQL_MESSAGE_H
#define LIB_NETSOCKET_SQL_MESSAGE_H

#include <string>

//type of sql action to do
enum class sql_action_t
{
  none,
  create_table_places,
  create_table_items,
  insert_place,
  insert_item,
  get_places,
  get_items,
  select_places,
  select_items,
  all
};

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


#endif