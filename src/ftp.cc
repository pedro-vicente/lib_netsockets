#if defined (_MSC_VER)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> //hostent
#endif

#include <iostream>
#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "socket.hh"
#include "ftp.hh"

//FTP uses two TCP connections to transfer files : a control connection and a data connection
//connect a socket(control socket) to a ftp server on the port 21
//receive on the socket a message from the ftp server(code : 220)
//send login to the ftp server using the command USER and wait for confirmation (331)
//send password using the command PASS and wait for confirmation that you are logged on the server (230)
//send file:
//use the passive mode: send command PASV
//receive answer with an IP address and a port (227), parse this message.
//connect a second socket(a data socket) with the given configuration
//use the command STOR on the control socket
//send data through the data socket, close data socket.
//leave session using on the control socket the command QUIT.

/////////////////////////////////////////////////////////////////////////////////////////////////////
//ftp_t::ftp_t()
/////////////////////////////////////////////////////////////////////////////////////////////////////

ftp_t::ftp_t(const char *host_name, const unsigned short server_port)
  : tcp_client_t(host_name, server_port),
  m_server_port(server_port),
  sock_ctrl(0),
  sock_data(0)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//ftp_t::~ftp_t()
/////////////////////////////////////////////////////////////////////////////////////////////////////

ftp_t::~ftp_t()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//ftp_t::login()
/////////////////////////////////////////////////////////////////////////////////////////////////////

void ftp_t::login(const char *user_name, const char *pass)
{
  char buf_request[255];
  std::string str_server_ip;
  std::string str_rsp;
 
  //create the control socket
  create_socket(sock_ctrl, m_server_ip.c_str(), m_server_port);

  get_response(sock_ctrl, str_rsp);

  //construct USER request message using command line parameters
  //Note: there is no space between the user name and CRLF; example of request is "USER me\r\n"
  sprintf(buf_request, "USER %s\r\n", user_name);

  //send USER request 
  send_request(sock_ctrl, buf_request);

  //receive response 
  get_response(sock_ctrl, str_rsp);

  //construct PASS request message using command line parameters
  sprintf(buf_request, "PASS %s\r\n", pass);

  //send PASS request 
  send_request(sock_ctrl, buf_request);

  //receive response 
  get_response(sock_ctrl, str_rsp);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//ftp_t::logout()
/////////////////////////////////////////////////////////////////////////////////////////////////////

void ftp_t::logout()
{
  char buf_request[255];
  std::string str_rsp;

  //construct QUIT request message 
  sprintf(buf_request, "QUIT\r\n");

  //send QUIT request (on control socket)
  send_request(sock_ctrl, buf_request);

  //get 'Goodbye' response
  get_response(sock_ctrl, str_rsp);

  close_socket(sock_ctrl);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//ftp_t::get_file_list()
/////////////////////////////////////////////////////////////////////////////////////////////////////

void ftp_t::get_file_list()
{
  char buf_request[255];
  std::string str_server_ip;
  std::string str_rsp;
  unsigned short server_port = 21;

  //enter passive mode: make PASV request
  sprintf(buf_request, "PASV\r\n");

  //send PASV request 
  send_request(sock_ctrl, buf_request);

  //receive response 
  get_response(sock_ctrl, str_rsp);

  //parse the PASV response
  parse_PASV_response(str_rsp, str_server_ip, server_port);

  //create the data socket
  create_socket(sock_data, str_server_ip.c_str(), server_port);

  //construct NLST (list) request message 
  sprintf(buf_request, "NLST\r\n");

  //send NLST request (on control socket)
  send_request(sock_ctrl, buf_request);

  //get response on control socket
  get_response(sock_ctrl, str_rsp);

  //get list on the data socket
  receive_list(sock_data);

  //get response on control socket
  get_response(sock_ctrl, str_rsp);

  //close data socket
  close_socket(sock_data);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//ftp_t::get_file()
/////////////////////////////////////////////////////////////////////////////////////////////////////

void ftp_t::get_file(const char *file_name)
{
  char buf_request[255];
  std::string str_server_ip;
  std::string str_rsp;
  unsigned short server_port;

  //The FTP command, SIZE OF FILE (SIZE), is used to obtain the transfer
  //size of a file from the server-FTP process.This is the exact number
  //of octets(8 bit bytes) that would be transmitted over the data
  //connection should that file be transmitted.
  //syntax: "SIZE" SP pathname CRLF
  //get SIZE
  sprintf(buf_request, "SIZE %s\r\n", file_name);

  //send SIZE request 
  send_request(sock_ctrl, buf_request);

  //get response on control socket
  get_response(sock_ctrl, str_rsp);

  //parse the file size
  std::string  str_code = str_rsp.substr(0, 3);
  unsigned long long size_file = 0;
  if ("213" == str_code)
  {
    std::string  str_size = str_rsp.substr(4, str_rsp.size() - 2 - 4); //subtract end CRLF plus start of "213 ", 213 SP
    size_file = std::stoull(str_size);

    std::cout << "FILE transfer is " << size_file << " bytes" << std::endl;
  }

  //enter passive mode: make PASV request
  sprintf(buf_request, "PASV\r\n");

  //send PASV request 
  send_request(sock_ctrl, buf_request);

  //get response on control socket
  get_response(sock_ctrl, str_rsp);

  //parse the PASV response
  parse_PASV_response(str_rsp, str_server_ip, server_port);

  //create the data socket
  create_socket(sock_data, str_server_ip.c_str(), server_port);

  //construct RETR request message 
  sprintf(buf_request, "RETR %s\r\n", file_name);

  //send RETR request on control socket
  send_request(sock_ctrl, buf_request);

  //get response on control socket
  get_response(sock_ctrl, str_rsp);

  //get the file (data socket), save to local file with same name
  receive_all(sock_data, file_name);

  //get response
  get_response(sock_ctrl, str_rsp);

  //close data socket
  close_socket(sock_data);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//ftp_t::receive_list
/////////////////////////////////////////////////////////////////////////////////////////////////////

void ftp_t::receive_list(int sock)
{
  int recv_size; // size in bytes received or -1 on error 
  const int flags = 0;
  const int size_buf = 255;
  char buf[size_buf];
  std::string str_nlst;

  while (1)
  {
    if ((recv_size = recv(sock, buf, size_buf, flags)) == -1)
    {
      std::cout << "recv error: " << strerror(errno) << std::endl;
      exit(1);
    }
    if (recv_size == 0)
    {
      std::cout << "all bytes received " << std::endl;
      break;
    }
    for (int i = 0; i < recv_size; i++)
    {
      str_nlst += buf[i];
    }
  }

  if (!str_nlst.size())
  {
    return;
  }

  //parse file list into a vector
  size_t start = 0;
  size_t count = 0;
  for (size_t idx = 0; idx < str_nlst.size() - 1; idx++)
  {
    //detect CRLF
    if (str_nlst.at(idx) == '\r' && str_nlst.at(idx + 1) == '\n')
    {
      count = idx - start;
      std::string str = str_nlst.substr(start, count);
      start = idx + 2;
      m_file_nslt.push_back(str);
      std::cout << str << std::endl;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//ftp_t::create_socket
/////////////////////////////////////////////////////////////////////////////////////////////////////

void ftp_t::create_socket(int &sock, const char* server_ip, const unsigned short server_port)
{
  // server address
  struct sockaddr_in server_addr;

  // construct the server address structure
  memset(&server_addr, 0, sizeof(server_addr));// zero out structure
  server_addr.sin_family = AF_INET;// internet address family
  server_addr.sin_addr.s_addr = inet_addr(server_ip);// server IP address
  server_addr.sin_port = htons(server_port);// server port

  // create a stream socket using TCP
  if ((sock = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
  {
  }

  // establish the connection to the server
  if (::connect(sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
  {
    std::cout << "connect error: " << strerror(errno) << std::endl;
    exit(1);
  }
}

///////////////////////////////////////////////////////////////////////////////////////
//ftp_t::get_response()
///////////////////////////////////////////////////////////////////////////////////////

void ftp_t::get_response(int sock, std::string &str_rep)
{
  int recv_size; // size in bytes received or -1 on error 
  const int flags = 0;
  const int size_buf = 1024;
  char buf[size_buf];

  if ((recv_size = recv(sock, buf, size_buf, flags)) == -1)
  {
    std::cout << "recv error: " << strerror(errno) << std::endl;
  }

  std::string str(buf, recv_size);
  std::cout << "response: " << str;
  str_rep = str;
  return;
}

///////////////////////////////////////////////////////////////////////////////////////
//ftp_t::send_request()
///////////////////////////////////////////////////////////////////////////////////////

void ftp_t::send_request(int sock, const char* buf_request)
{
  std::cout << "request: " << buf_request << std::endl;

  //send 
  send_all(sock, (void *)buf_request, strlen(buf_request));
  return;
}

///////////////////////////////////////////////////////////////////////////////////////
//ftp_t::parse_PASV_response()
//PASV request asks the server to accept a data connection on a new TCP port selected by the server
//PASV parameters are prohibited
//The server normally accepts PASV with code 227
//Its response is a single line showing the IP address of the server and the TCP port number 
//where the server is accepting connections
//RFC 959 failed to specify details of the response format. 
//implementation examples
//227 Entering Passive Mode (h1,h2,h3,h4,p1,p2).
//the TCP port number is p1*256+p2
///////////////////////////////////////////////////////////////////////////////////////

void ftp_t::parse_PASV_response(const std::string &str_rsp, std::string &str_server_ip, unsigned short &server_port)
{
  unsigned int h[4];
  unsigned int p[2];
  char server_ip[100];

  size_t pos = str_rsp.find('(');
  std::string  str_ip = str_rsp.substr(pos + 1);
  sscanf(str_ip.c_str(), "%u,%u,%u,%u,%u,%u", &h[0], &h[1], &h[2], &h[3], &p[0], &p[1]);
  server_port = static_cast<unsigned short>(p[0] * 256 + p[1]);
  sprintf(server_ip, "%u.%u.%u.%u", h[0], h[1], h[2], h[3]);
  str_server_ip = server_ip;
}

///////////////////////////////////////////////////////////////////////////////////////
//ftp_t::close_socket
///////////////////////////////////////////////////////////////////////////////////////

void ftp_t::close_socket(int sock)
{
#if defined (_MSC_VER)
  ::closesocket(sock);
#else
  ::close(sock);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////
//receive_all()
//http://man7.org/linux/man-pages/man2/recv.2.html
//recv() 'size_buf' bytes and save to local FILE
//assumptions: connection was closed on the FTP server, allowing recv() to return 0
///////////////////////////////////////////////////////////////////////////////////////

void ftp_t::receive_all(int sock, const char *file_name)
{
  int recv_size; // size in bytes received or -1 on error 
  const int flags = 0;
  const int size_buf = 4096;
  char buf[size_buf];
  FILE *file;

  file = fopen(file_name, "wb");
  while (1)
  {
    if ((recv_size = recv(sock, buf, size_buf, flags)) == -1)
    {
      std::cout << "recv error: " << strerror(errno) << std::endl;
    }
    if (recv_size == 0)
    {
      std::cout << "all bytes received " << std::endl;
      break;
    }
    fwrite(buf, recv_size, 1, file);
  }
  fclose(file);
  return;
}

///////////////////////////////////////////////////////////////////////////////////////
//ftp_t::send_all()
//http://man7.org/linux/man-pages/man2/sendto.2.html
//send() 'size_buf' bytes
//TCP is a byte stream. There is no guarantee of a one to one relation between the number of 
//items sent and the number of calls to send() or recv().
//send_all() accepts a buffer size as parameter and keeps looping on a send() call, 
//(that returns the number of bytes sent) until all bytes are sent.
///////////////////////////////////////////////////////////////////////////////////////

void ftp_t::send_all(int sock, const void *vbuf, size_t size_buf)
{
  const char *buf = (char*)vbuf;	// can't do pointer arithmetic on void* 
  int send_size; // size in bytes sent or -1 on error 
  int size_left; // size left to send 
  const int flags = 0;

  size_left = size_buf;
  while (size_left > 0)
  {
    if ((send_size = send(sock, buf, size_left, flags)) == -1)
    {
      std::cout << "send error: " << std::endl;
    }
    size_left -= send_size;
    buf += send_size;
  }
  return;
}
