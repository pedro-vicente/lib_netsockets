#ifndef TLS_SOCKET_HH
#define TLS_SOCKET_HH 1

#if defined (_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/conf.h>
#include <openssl/x509.h>
#include <openssl/buffer.h>
#include <openssl/x509v3.h>
#include <openssl/opensslconf.h>
#include <string>

/////////////////////////////////////////////////////////////////////////////////////////////////////
//tls_socket_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

class tls_socket_t
{
public:
  tls_socket_t();
  ~tls_socket_t();
  int open(const char *host, const char* port);
  int send(const char *buf);
  int receive();
protected:
  SSL *m_ssl;
  BIO *m_socket;
  BIO *m_out;
  SSL_CTX* m_ctx;
};

#endif

