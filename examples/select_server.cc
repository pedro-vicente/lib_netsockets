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
  //an fd_set is a set of sockets to monitor for some activity. 
  //there are four useful macros : FD_CLR, FD_ISSET, FD_SET, FD_ZERO for dealing with an fd_set.
  //FD_ZERO. clear an fd_set
  //FD_ISSET. check if a descriptor is in an fd_set
  //FD_SET. add a descriptor to an fd_set
  //FD_CLR. remove a descriptor from an fd_set

  //set of socket descriptors, for read
  fd_set readfds;

  //add place for 5 clients
  std::vector<socket_t> clients;
  for (size_t idx = 0; idx < 5; idx++)
  {
    socket_t client;
    clients.push_back(client);
  }

  //create server socket, bind and listen
  tcp_server_t server(2000);

  //maximum descriptor for select
  socketfd_t max_fd = server.m_sockfd;

  while (true)
  {
    //clear the socket set
    FD_ZERO(&readfds);

    //add server socket to set
    FD_SET(server.m_sockfd, &readfds);

    //add child sockets (in the next iteration) to the readfds set
    for (size_t idx = 0; idx < clients.size(); idx++)
    {
      //existing socket descriptor
      socketfd_t fd = clients.at(idx).m_sockfd;

      //if filled socket descriptor then add to read list
      if (fd > 0)
      {
        FD_SET(fd, &readfds);
      }

      //highest file descriptor number, need it for the select function
      if (fd > max_fd)
      {
        max_fd = fd;
      }
    }

    //the select function blocks , until an activity occurs
    //when a socket is ready to be read , select will return and readfs will have those sockets 
    //which are ready to be read.
    //block until input arrives on one or more active sockets
    if (select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0)
    {
      perror("select");
      exit(EXIT_FAILURE);
    }

    //incoming connection on the server socket 
    if (FD_ISSET(server.m_sockfd, &readfds))
    {
      socket_t socket = server.accept();
      char* str_ip = inet_ntoa(socket.m_sockaddr_in.sin_addr);
      std::cout << prt_time() << "," << str_ip << "," << socket.m_sockfd << "\n";
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
      }
    }

    //data arriving on an already connected socket
    for (size_t idx = 0; idx < clients.size(); idx++)
    {
      socket_t socket = clients.at(idx);
      if (FD_ISSET(socket.m_sockfd, &readfds))
      {
        char buf[255];
        int size = socket.read_all(buf, sizeof(buf));
        std::string str(buf, size);
        std::cout << "received " << size << " bytes: " << str
          << " from socket " << socket.m_sockfd << "\n";
        //close connection (client must read all)
        socket.close();
        //mark it empty
        clients.at(idx).m_sockfd = 0;
      }
    }

  }//while
  server.close();
  return 0;
}



