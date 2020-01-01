#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <algorithm>
#include "socket.hh"

void usage()
{
  std::cout << "usage: /pub_sub_client <-s server_ip>" << std::endl;
  exit(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//subscriber_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

class subscriber_t
{
public:
  subscriber_t(const std::string& host_name_, const unsigned short server_port_, const std::string& name_id_) :
    host_name(host_name_),
    server_port(server_port_),
    name_id(name_id_)
  {};
  int update(const std::string& msg);
  tcp_client_t client;
  std::string host_name;
  unsigned short server_port;
  std::string name_id;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
//subscriber_t::update
//send a message 
/////////////////////////////////////////////////////////////////////////////////////////////////////

int subscriber_t::update(const std::string& msg)
{
  if (client.connect(host_name.c_str(), server_port) < 0)
  {
    return -1;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //construct message
  //header format:
  //5 bytes that describe the destination subscriber (e.g "sub01")
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  std::string msg_(name_id);
  msg_ += msg;
  if (client.write_all(msg_.c_str(), (int)msg_.size()) < 0)
  {
    return -1;
  }
  //close connection (server must read all)
  client.close();
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//publisher_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

class publisher_t
{
public:
  publisher_t() {}
  void notify(const subscriber_t& sub, const std::string& msg);
  std::vector<subscriber_t*> subscribers;
  void add(subscriber_t* sub);
  void remove(subscriber_t* sub);
};

void publisher_t::add(subscriber_t* sub)
{
  std::cout << "add " << sub->name_id << std::endl;
  subscribers.push_back(sub);
}

void publisher_t::remove(subscriber_t* sub)
{
  std::cout << "remove " << sub->name_id << std::endl;
  subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), sub), subscribers.end());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//publisher_t::notify
//send a message to a subscriber
/////////////////////////////////////////////////////////////////////////////////////////////////////

void publisher_t::notify(const subscriber_t& sub_, const std::string& msg)
{
  //iterate list of subscribers, send to the one that matches name id
  for (size_t idx = 0; idx < subscribers.size(); idx++)
  {
    subscriber_t* sub = subscribers.at(idx);
    if (sub_.name_id.compare(sub->name_id) == 0)
    {
      if (0) std::cout << "update " << sub->name_id << " with " << msg << std::endl;
      if (sub->update(msg) < 0)
      {

      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//main
/////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  wait(1);
  std::string host_name("127.0.0.1");
  size_t nbr_itr = 2;
  const unsigned short server_port_1 = 4000;
  const unsigned short server_port_2 = 5000;

  for (int i = 1; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      switch (argv[i][1])
      {
      case 's':
        host_name = argv[i + 1];
        i++;
        break;
      case 'n':
        nbr_itr = std::stoi(argv[i + 1]);
        i++;
        break;
      default:
        usage();
      }
    }
  }

  publisher_t pub;
  subscriber_t sub1(host_name, server_port_1, "sub01");
  subscriber_t sub2(host_name, server_port_2, "sub02");

  pub.add(&sub1);
  pub.add(&sub2);
  for (size_t i = 0; i < nbr_itr; i++)
  {
    pub.notify(sub1, "messA");
    pub.notify(sub2, "messB");
    std::cout << i + 1 << " ";
  }
  return 0;
}

