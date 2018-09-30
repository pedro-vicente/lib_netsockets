#include "tls_socket.hh"
#include <fstream>
#include <iostream>

const char* const PREFERRED_CIPHERS = "HIGH:!aNULL:!kRSA:!SRP:!PSK:!CAMELLIA:!RC4:!MD5:!DSS";

/////////////////////////////////////////////////////////////////////////////////////////////////////
//tls_socket_t::tls_socket_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

tls_socket_t::tls_socket_t() :
  m_ssl(NULL),
  m_socket(NULL),
  m_out(NULL)
{
  unsigned long ssl_err = 0;
#if defined (_MSC_VER)
  WSADATA ws_data;
  if (WSAStartup(MAKEWORD(2, 0), &ws_data) != 0)
  {
    exit(1);
  }
#endif
  (void)SSL_library_init();
  SSL_load_error_strings();
  OPENSSL_config(NULL);
  const SSL_METHOD* method = SSLv23_method();
  ssl_err = ERR_get_error();
  m_ctx = SSL_CTX_new(method);
  ssl_err = ERR_get_error();
  const long flags = SSL_OP_ALL | SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
  SSL_CTX_set_options(m_ctx, flags);
  SSL_CTX_load_verify_locations(m_ctx, "random-org-chain.pem", NULL);
  ssl_err = ERR_get_error();
  m_socket = BIO_new_ssl_connect(m_ctx);
  ssl_err = ERR_get_error();
  assert(m_socket != NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//tls_socket_t::~tls_socket_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

tls_socket_t::~tls_socket_t()
{
  if (m_out)
  {
    BIO_free(m_out);
  }
  if (m_socket != NULL)
  {
    BIO_free_all(m_socket);
  }
  if (NULL != m_ctx)
  {
    SSL_CTX_free(m_ctx);
  }
#if defined (_MSC_VER)
  WSACleanup();
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//tls_socket_t::open
/////////////////////////////////////////////////////////////////////////////////////////////////////

int tls_socket_t::open(const char *host_name)
{
  long res = 1;
  unsigned long ssl_err = 0;
  std::string name(host_name);
  name += ":443";
  BIO_set_conn_hostname(m_socket, name.c_str());
  ssl_err = ERR_get_error();
  BIO_get_ssl(m_socket, &m_ssl);
  ssl_err = ERR_get_error();
  assert(m_ssl != NULL);
  SSL_set_cipher_list(m_ssl, PREFERRED_CIPHERS);
  ssl_err = ERR_get_error();
  SSL_set_tlsext_host_name(m_ssl, host_name);
  ssl_err = ERR_get_error();
  m_out = BIO_new_fp(stdout, BIO_NOCLOSE);
  ssl_err = ERR_get_error();
  assert(NULL != m_out);
  BIO_do_connect(m_socket);
  ssl_err = ERR_get_error();
  res = BIO_do_handshake(m_socket);
  ssl_err = ERR_get_error();
  assert(1 == res);
  X509* cert = SSL_get_peer_certificate(m_ssl);
  if (cert)
  {
    X509_free(cert);
  }
  assert(NULL != cert);
  res = SSL_get_verify_result(m_ssl);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//tls_socket_t::send
/////////////////////////////////////////////////////////////////////////////////////////////////////

int tls_socket_t::send(const char *buf)
{
  std::cout << buf;
  BIO_puts(m_socket, buf);
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//tls_socket_t::receive
/////////////////////////////////////////////////////////////////////////////////////////////////////

int tls_socket_t::receive()
{
  std::string response;
  int len = 0;
  do {
    char buf[1024] = {};
    len = BIO_read(m_socket, buf, sizeof(buf));
    if (len > 0)
    {
      BIO_write(m_out, buf, len);
      std::string str(buf);
      response += str;
    }
  } while (len > 0 || BIO_should_retry(m_socket));
  std::ofstream ofs("response.txt", std::ios::out | std::ios::binary);
  ofs.write(response.c_str(), response.size());
  ofs.close();
  return 0;
}


