#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "socket.hh"

///////////////////////////////////////////////////////////////////////////////////////
//main
//example server using ::select
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
  //add place for 5 clients
  std::vector<socket_t> clients;
  for (size_t idx = 0; idx < 5; idx++)
  {
    socket_t client;
    clients.push_back(client);
  }

  //an fd_set is a set of sockets to monitor for some activity. 
  //there are four useful macros : FD_CLR, FD_ISSET, FD_SET, FD_ZERO for dealing with an fd_set.
  //FD_ZERO. clear an fd_set
  //FD_ISSET. check if a descriptor is in an fd_set
  //FD_SET. add a descriptor to an fd_set
  //FD_CLR. remove a descriptor from an fd_set

  //set of socket descriptors, for read
  fd_set read_fds;
  //active socket descriptors
  fd_set active_fds;

  //create server socket, bind and listen
  tcp_server_t server(2000);

  //maximum descriptor for select
  socketfd_t max_fd = server.m_sockfd;

  //initialize the set of active sockets
  FD_ZERO(&active_fds);

  //add server socket to set
  FD_SET(server.m_sockfd, &active_fds);

  while (true)
  {
    read_fds = active_fds;

    //the select function blocks , until an activity occurs
    //when a socket is ready to be read , select will return and read_fds will have those sockets 
    //which are ready to be read.
    //select modifies read_fds
    if (select((int)max_fd + 1, &read_fds, NULL, NULL, NULL) < 0)
    {
      perror("select");
      exit(EXIT_FAILURE);
    }

    //one or more descriptors are readable, need to determine which ones they are
    for (int idx_act = 0; idx_act <= max_fd; idx_act++)
    {
      //check to see if this descriptor is ready
      if (FD_ISSET(idx_act, &read_fds))
      {
        std::cout << "descriptor " << idx_act << " ready\n";

        ///////////////////////////////////////////////////////////////////////////////////////
        //check to see if this is the listening socket
        ///////////////////////////////////////////////////////////////////////////////////////

        if (idx_act == server.m_sockfd)
        {
          std::cout << "listening socket " << idx_act << " ready\n";
          socket_t socket = server.accept();
          char* str_ip = inet_ntoa(socket.m_sockaddr_in.sin_addr);
          std::cout << "accepted at " << prt_time() << "," << str_ip
            << "," << socket.m_sockfd << "\n";

          //add the new incoming connection to the master read set
          FD_SET(socket.m_sockfd, &active_fds);
          if (socket.m_sockfd > max_fd)
          {
            max_fd = socket.m_sockfd;
          }

          //add new socket to array of sockets  
          for (size_t idx = 0; idx < clients.size(); idx++)
          {
            //if position is empty, add it to list of sockets
            if (clients.at(idx).m_sockfd == 0)
            {
              clients.at(idx) = socket;
              std::cout << "added descriptor " << clients.at(idx).m_sockfd
                << " at position " << idx << "\n";
              break;
            }
          }//add new socket
        } //listening socket

        ///////////////////////////////////////////////////////////////////////////////////////
        //data arriving on an already-connected socket
        //search in the list of connected clients
        ///////////////////////////////////////////////////////////////////////////////////////

        else
        {
          for (size_t idx = 0; idx < clients.size(); idx++)
          {
            if (clients.at(idx).m_sockfd == idx_act)
            {
              socket_t socket = clients.at(idx);
              char buf[255];
              int size = socket.read_all(buf, sizeof(buf));
              std::string str(buf, size);
              std::cout << "received " << size << " bytes: " << str
                << " from socket " << socket.m_sockfd << "\n\n";
              //close connection (client must read all) and mark it empty
              clients.at(idx).close();
              //clear from active set
              FD_CLR(idx_act, &active_fds);
            }//socket.m_sockfd
          }//clients.size()

        }//data arriving on an already-connected socket
      }//FD_ISSET
    } //for max_fd
  }//while
  server.close();
  return 0;
}
