# lib_netsockets
C++ light wrapper for POSIX and Winsock sockets using TCP. Examples include:
<br /> 
* TCP client/server using custom JSON messages
* TCP client/server sending SQL as JSON to SQLite database in TCP server
* HTTP client/server
* FTP client

Dependencies 
------------

[gason](https://github.com/vivkin/gason) (included)
<br /> 

[SQLite](https://www.sqlite.org/) (included)
<br />

Building
------------

## Building in Linux 

Install dependencies

[cmake](https://cmake.org/)

Install packages with

```
sudo apt-get install cmake
```

Get source:
<pre>
git clone https://github.com/pedro-vicente/lib_netsockets.git
cd build
cmake ..
make
</pre>

## Building in Windows 
<pre>
bld.bat
</pre>

# Usage
lib_netsockets is C++ light wrapper for POSIX and Winsock sockets with implementation of TCP client/server using JSON messages,and HTTP, FTP clients.

# TCP server example
```c++
char buf[255];
tcp_server_t server(2000);
while (true)
{
 socket_t socket = server.accept();
 int size = socket.read_all(buf, sizeof(buf));
 socket.close();
}
server.close();
```

# TCP client example
```c++
char buf[20];
sprintf(buf, "1234");
tcp_client_t client("127.0.0.1", 2000);
client.connect();
client.write_all(buf, strlen(buf));
client.close();
```

# HTTP client example
```c++
http_t client("www.mysite.com", 80);
client.get("/my/path/to/file");
```

# FTP client example
Get file list from FTP server and first file in list
```c++
ftp_t ftp("my.ftp.site", 21);
ftp.login("my user", "anonymous");
ftp.get_file_list();
ftp.get_file(ftp.m_file_nslt.at(0).c_str());
ftp.logout();
```

# SQLite server/client example
This example provides a REST API to send SQL commands as a JSON array from a TCP client to TCP server.
The example consists of a SQLite database with 2 tables "places" (like your favorite places) and "items" (like items for each place).
This database resides on the server. API is

```
/get_places
/get_items
/create_table_places
/create_table_items
/insert_place
/insert_item
/select_places
/select_items
```

Example call:

```
./sqlite_client -u http://127.0.0.1/create_table_places
```

sqlite_client generates the following SQL, and encodes it in JSON, sending it as a HTTP POST

```
CREATE TABLE IF NOT EXISTS table_places(place_id TEXT PRIMARY KEY NOT NULL,address CHAR(50) NOT NULL,rank INTEGER NOT NULL);
INSERT INTO table_places VALUES('home', '102 E. Green St. Urbana IL 61801', 1);
SELECT * FROM table_places WHERE place_id = 'home';
```

HTTP GET calls are 

```
/get_places
/get_items
```

These can be done in a browser, e.g

```
http://127.0.0.1/get_places
```

will cause the server to send all rows from table "places" to the browser as a HTTP json response

# TCP JSON server/client output
```
server listening on port 2001
Wed May 16 13:53:21 2018 server accepted: 127.0.0.1 <260>
Wed May 16 13:53:21 2018 server received: {"start_year":2017}
Wed May 16 13:53:21 2018 server sent: {"next_year":2018}
```

# HTTP client usage
```
usage: http_client -s SERVER -t 'HTTP_REQUEST' <-p PORT> <-v> <-h>
-s SERVER: fully qualified web server name
-p PORT: server port (default 80)
-t 'HTTP_REQUEST', string enquoted
-v: verbose, output of retrieved file is printed
-h: help, exit
```

# HTTP REST example request
Example REST API

https://jsonplaceholder.typicode.com/

```
./http_client -s jsonplaceholder.typicode.com -t "GET  /posts/1"
```

HTTP request

```
GET  /posts/1 HTTP/1.1
Host: jsonplaceholder.typicode.com
Accept: application/json
Connection: close
```


HTTP response

```
HTTP/1.1 200 OK
Date: Wed, 16 May 2018 22:34:45 GMT
Content-Type: application/json; charset=utf-8
Content-Length: 292
Connection: close
Set-Cookie: __cfduid=d1f77010e32b4333f04dff766134fd4eb1526510085; expires=Thu, 16-May-19 22:34:45 GMT; path=/; domain=.typicode.com; HttpOnly
X-Powered-By: Express
Vary: Origin, Accept-Encoding
Access-Control-Allow-Credentials: true
Cache-Control: public, max-age=14400
Pragma: no-cache
Expires: Thu, 17 May 2018 02:34:45 GMT
X-Content-Type-Options: nosniff
Etag: W/"124-yiKdLzqO5gfBrJFrcdJ8Yq0LGnU"
Via: 1.1 vegur
CF-Cache-Status: HIT
Server: cloudflare
CF-RAY: 41c1503f5416241a-IAD

{
  "userId": 1,
  "id": 1,
  "title": "sunt aut facere repellat provident occaecati excepturi optio reprehenderit",
  "body": "quia et suscipit\nsuscipit recusandae consequuntur expedita et cum\nreprehenderit molestiae ut ut quas totam\nnostrum rerum est autem sunt rem eveniet architecto"
}
```