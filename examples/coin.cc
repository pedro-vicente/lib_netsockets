#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "coin.hh"


/////////////////////////////////////////////////////////////////////////////////////////////////////
//currency_t::currency_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

currency_t::currency_t()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//currency_t::get_currencies_list
/////////////////////////////////////////////////////////////////////////////////////////////////////

int currency_t::get_currencies_list(const char* fname)
{
  char *buf = 0;
  size_t length;
  FILE *f;

  f = fopen(fname, "rb");
  if (!f)
  {
    std::cout << "cannot open " << fname << std::endl;
    return -1;
  }

  fseek(f, 0, SEEK_END);
  length = ftell(f);
  fseek(f, 0, SEEK_SET);
  buf = (char*)malloc(length);
  if (buf)
  {
    fread(buf, 1, length, f);
  }
  fclose(f);

  char *endptr;
  JsonValue value;
  JsonAllocator allocator;
  int status = jsonParse(buf, &endptr, &value, allocator);
  if (status != JSON_OK)
  {
    std::cout << "invalid JSON format for " << fname << std::endl;
    return -1;
  }

  JsonNode *root_obj = value.toNode();
  //format is "coins" JSON object with an array of objects
  assert(std::string(root_obj->key).compare("coins") == 0);
  assert(root_obj->value.getTag() == JSON_ARRAY);
  JsonNode *arr = root_obj->value.toNode();

  size_t nbr_curr = 0;
  for (JsonNode *node = arr; node != nullptr; node = node->next)
  {
    assert(node->value.getTag() == JSON_OBJECT);
    parse_coin(node->value);
    nbr_curr++;
  }

  assert(nbr_curr == m_coin_list.size());
  std::cout << "parsed " << nbr_curr << " currencies..." << std::endl;
  free(buf);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//currency_t::parse_coin
//object with {"id":2024,"name":"007Coin","code":"007"}
/////////////////////////////////////////////////////////////////////////////////////////////////////

int currency_t::parse_coin(JsonValue value)
{
  assert(value.getTag() == JSON_OBJECT);
  coin_list_t coin;
  for (JsonNode *node = value.toNode(); node != nullptr; node = node->next)
  {
    if (std::string(node->key).compare("id") == 0)
    {
      assert(node->value.getTag() == JSON_NUMBER);
      coin.id = (int)node->value.toNumber();
    }
    else if (std::string(node->key).compare("name") == 0)
    {
      assert(node->value.getTag() == JSON_STRING);
      coin.name = node->value.toString();
    }
    else if (std::string(node->key).compare("code") == 0)
    {
      assert(node->value.getTag() == JSON_STRING);
      coin.code = node->value.toString();
    }
  }
  m_coin_list.push_back(coin);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//currency_t::get_coin_id
/////////////////////////////////////////////////////////////////////////////////////////////////////

std::string currency_t::get_coin_id(std::string code)
{
  std::string str;
  size_t size = m_coin_list.size();
  for (size_t idx = 0; idx < size; idx++)
  {
    if (std::string(code).compare(m_coin_list.at(idx).code) == 0)
    {
      return std::to_string(m_coin_list.at(idx).id);
    }
  }
  return str;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//currency_t::get_history
/////////////////////////////////////////////////////////////////////////////////////////////////////

int currency_t::get_history(char* buf)
{
  char *endptr;
  JsonValue value;
  JsonAllocator allocator;
  int status = jsonParse(buf, &endptr, &value, allocator);
  if (status != JSON_OK)
  {
    std::cout << "invalid JSON format" << std::endl;
    return -1;
  }

  JsonNode *root = value.toNode();
  std::string code = root->value.toNode()->next->next->value.toString();
  coin_history_t history(code);
  JsonNode *arr = root->next->next->next->value.toNode();
  size_t nbr_hist = 0;
  for (JsonNode *node = arr; node != nullptr; node = node->next)
  {
    assert(node->value.getTag() == JSON_OBJECT);
    std::string date = node->value.toNode()->value.toString();
    std::string price = node->value.toNode()->next->value.toString();
    history.m_history.push_back(date_price_t(date, price));
    nbr_hist++;
  }

  m_coin_history.push_back(history);
  return 0;
}

