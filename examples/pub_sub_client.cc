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
  std::cout << "usage: /pub_sub_client -s server_ip" << std::endl;
  exit(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//subscriber_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

class subscriber_t
{
public:
  subscriber_t(const std::string& name_id_) :
    name_id(name_id_)
  {};
  void update(const std::string& msg);
  std::string name_id;

  tcp_client_t client;
};

//send a message 
void subscriber_t::update(const std::string& msg)
{

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

int main()
{
  std::string host_name("127.0.0.1");
  publisher_t pub;
  subscriber_t sub1("sub1");
  subscriber_t sub2("sub2");

  pub.add(&sub1);
  pub.add(&sub2);
  pub.notify(sub1, "message A");
  pub.notify(sub2, "message B");
  pub.remove(&sub1);
  pub.notify(sub1, "message A");
  pub.notify(sub2, "message B");
  return 0;
}

