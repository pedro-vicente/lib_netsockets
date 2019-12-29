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
  void update(const std::string& msg);
  tcp_client_t client;
  std::string host_name;
  unsigned short server_port;
  std::string name_id;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////
//subscriber_t::update
//send a message 
/////////////////////////////////////////////////////////////////////////////////////////////////////

void subscriber_t::update(const std::string& msg)
{
  if (client.connect(host_name.c_str(), server_port) < 0)
  {
    std::cout << "no connection to... " << host_name.c_str() << "\n";
    return;
  }
  std::cout << "client connected  " << client.m_sockfd << "\n";

  if (client.write_all(msg.c_str(), (int)msg.size()) < 0)
  {
    client.close();
    return;
  }
  std::cout << "client sent " << msg.size() << " bytes\n";

  //close connection (server must read all)
  client.close();
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

void publisher_t::notify(const subscriber_t& sub, const std::string& msg)
{
  //iterate list of subscribers, send to the one that matches name id
  for (size_t idx = 0; idx < subscribers.size(); idx++)
  {
    subscriber_t* sub_ = subscribers.at(idx);
    if (sub.name_id.compare(sub_->name_id) == 0)
    {
      std::cout << "update " << sub_->name_id << " with " << msg << std::endl;
      sub_->update(msg);
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
      default:
        usage();
      }
    }
  }


  publisher_t pub;
  subscriber_t sub1(host_name, server_port_1, "sub1");
  subscriber_t sub2(host_name, server_port_2, "sub2");

  pub.add(&sub1);
  pub.add(&sub2);
  pub.notify(sub1, "message A");
  pub.notify(sub2, "message B");
  pub.remove(&sub2);
  pub.notify(sub1, "message A");
  pub.notify(sub2, "message B");
  return 0;
}

