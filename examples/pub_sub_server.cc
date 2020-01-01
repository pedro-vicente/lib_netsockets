#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "socket.hh"

int parse_header(socketfd_t fd, std::string& header);

/////////////////////////////////////////////////////////////////////////////////////////////////////
//topic_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

class topic_t
{
public:
  topic_t(const std::string& name_id_) :
    name_id(name_id_),
    nbr_recv(0),
    size_recv(0)
  {};
  std::string name_id;
  size_t nbr_recv;
  size_t size_recv;
  int read(socket_t& socket);
};

///////////////////////////////////////////////////////////////////////////////////////
//usage
///////////////////////////////////////////////////////////////////////////////////////

void usage()
{
  std::cout << "usage: /pub_sub_server <-p port>" << std::endl;
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//main
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  unsigned short server_port = 4000;
  std::vector<topic_t> topics;

  for (int i = 1; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      switch (argv[i][1])
      {
      case 'p':
        server_port = std::stoi(argv[i + 1]);
        i++;
        break;
      default:
        usage();
      }
    }
  }

  tcp_server_t server(server_port);
  std::cout << "server listening\n";

  while (true)
  {
    socket_t socket = server.accept();
    char* str_ip = inet_ntoa(socket.m_sockaddr_in.sin_addr);

    std::string name_id;
    if (parse_header(socket.m_sockfd, name_id) <= 0)
    {
      continue;
    }

    int found = 0;
    for (size_t idx = 0; idx < topics.size(); idx++)
    {
      if (topics.at(idx).name_id.compare(name_id) == 0)
      {
        found = 1;
        topics.at(idx).read(socket);
        continue;
      }
    }
    if (found == 0)
    {
      topic_t topic(name_id);
      topic.read(socket);
      topics.push_back(topic);
    }
  }
  //not reached 
  server.close();
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//parse_header
//return value
//0 nothing received
//1 something received
//-1 error
//header format:
//5 bytes that describe the destination subscriber (e.g "sub01")
/////////////////////////////////////////////////////////////////////////////////////////////////////

int parse_header(socketfd_t fd, std::string& header)
{
  int recv_size; // size in bytes received or -1 on error 
  const int size_buf = 5;
  char buf[size_buf];

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  //MSG_PEEK
  //Peeks at an incoming message.The data is treated as unread and the next recv() or similar 
  //function shall still return this data.
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  if ((recv_size = ::recv(fd, buf, size_buf, MSG_PEEK)) == -1)
  {
    std::cout << "recv error: " << strerror(errno) << std::endl;
    return -1;
  }

  if (recv_size == 0)
  {
    return 0;
  }

  std::string str(buf);
  header = str;
  return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//topic_t::read
/////////////////////////////////////////////////////////////////////////////////////////////////////

int topic_t::read(socket_t& socket)
{
  char buf[255];
  std::ofstream ofs;

  int size = socket.read_all(buf, sizeof(buf));
  nbr_recv++;
  size_recv += size;
  if (1)
  {
    ofs.open(name_id + ".log.txt", std::ofstream::out | std::ofstream::app);
    std::string str = std::to_string(nbr_recv);
    str += ",";
    str += std::to_string(size_recv);
    std::cout << str << " ";
    str += "\n";
    ofs.write(str.c_str(), str.size());
    ofs.close();
  }

  //close connection
  socket.close();
  return size;
}

