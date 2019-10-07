#ifndef LIB_NETSOCKET_HTTP_H
#define LIB_NETSOCKET_HTTP_H
#include <string>
#include "socket.hh"

int parse_http_headers(socketfd_t fd, std::string& header);
unsigned long long http_get_field(const std::string& str_field, const std::string& str_header);
std::string http_get_body(const std::string& str_header);
std::string http_get_method(const std::string& str_header);
std::string escape_space(const std::string& str);

/////////////////////////////////////////////////////////////////////////////////////////////////////
//http_client_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

class http_client_t : public tcp_client_t
{
public:
  http_client_t(const char *host_name, const unsigned short server_port);
  int get(const char *path_remote_file);
  int post(const std::string& str_body);
};




#endif
