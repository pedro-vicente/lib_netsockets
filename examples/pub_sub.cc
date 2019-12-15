#include <vector>
#include <algorithm>
#include <string>
#include <iostream>

/////////////////////////////////////////////////////////////////////////////////////////////////////
//subscriber_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

class subscriber_t
{
public:
  subscriber_t(const std::string& name_) :
    name(name_)
  {};
  void update(const std::string& msg);
  std::string message;
  std::string name;
};

void subscriber_t::update(const std::string& msg)
{
  message = msg;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//publisher_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

class publisher_t
{
public:
  publisher_t() {}
  void notify(const std::string& msg);
  std::vector<subscriber_t*> subscribers;
  void add(subscriber_t* sub);
  void remove(subscriber_t* sub);
};

void publisher_t::add(subscriber_t* sub)
{
  std::cout << "add " << sub->name << std::endl;
  subscribers.push_back(sub);
}

void publisher_t::remove(subscriber_t* sub)
{
  std::cout << "remove " << sub->name << std::endl;
  subscribers.erase(std::remove(subscribers.begin(), subscribers.end(), sub), subscribers.end());
}

void publisher_t::notify(const std::string& msg)
{
  for (size_t idx = 0; idx < subscribers.size(); idx++)
  {
    subscriber_t* sub = subscribers.at(idx);
    std::cout << "update " << sub->name << " with " << msg << std::endl;
    sub->update(msg);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//main
/////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
  publisher_t pub;
  subscriber_t sub1("sub1");
  subscriber_t sub2("sub2");

  pub.add(&sub1);
  pub.add(&sub2);
  pub.notify("message A");
  pub.remove(&sub1);
  pub.notify("message B");
  return 0;
}

