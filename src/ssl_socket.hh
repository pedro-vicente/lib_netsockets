#ifndef SSL_SOCKET_HH
#define SSL_SOCKET_HH 1

#if defined (_MSC_VER)
#undef UNICODE
#include <winsock2.h>
#include <ws2tcpip.h>
// link with Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> //hostent
#endif
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <string>

/////////////////////////////////////////////////////////////////////////////////////////////////////
//ssl_socket_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

class ssl_socket_t
{
public:
  ssl_socket_t();
  ~ssl_socket_t();
  int open(const char *str_ip);
  void close_socket();
  int send(const char *buf);
  int receive();

  //buffer to store response
  std::string m_response;

protected:
  int parse_http_headers();
  SSL *m_ssl;
  int m_socket_fd;
};

#endif

