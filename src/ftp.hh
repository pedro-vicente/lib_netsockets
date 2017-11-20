#ifndef LIB_NETSOCKET_FTP_H
#define LIB_NETSOCKET_FTP_H
#include <string>
#include <vector>
#include "socket.hh"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//ftp_t
/////////////////////////////////////////////////////////////////////////////////////////////////////

class ftp_t : public tcp_client_t
{
public:
  ftp_t(const char *host_name, const unsigned short server_port);
  ~ftp_t();
  void login(const char *user_name, const char *pass);
  void logout();
  void get_file_list();
  void get_file(const char *file_name);

  //file list on ftp server
  std::vector<std::string> m_file_nslt;

private:
  unsigned short m_server_port;
  int sock_ctrl; // control socket 
  int sock_data; // data socket 
  void receive_list(int sock);
  void get_response(int sock, std::string &str_rep);
  void send_request(int sock, const char* buf_request);
  void create_socket(int &sock, const char* server_ip, const unsigned short server_port);
  void parse_PASV_response(const std::string &str_rsp, std::string &str_server_ip, unsigned short &server_port);
  void close_socket(int sock);
  void receive_all(int sock, const char *file_name);
  void send_all(int sock, const void *vbuf, size_t size_buf);
};




#endif

