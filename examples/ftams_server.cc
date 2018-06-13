#include <string>
#include <iostream>
#include <stdio.h>
#include <string.h>


/////////////////////////////////////////////////////////////////////////////////////////////////////
//usage
//File Transfer Access Management Services
/////////////////////////////////////////////////////////////////////////////////////////////////////

bool verbose = true;

void usage()
{
  std::cout << "-p PORT: server port (default 4000)" << std::endl;
  std::cout << "-v: verbose output" << std::endl;
  std::cout << "-h: help, exit" << std::endl;
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////
//main
///////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
  unsigned short port = 4000;

  for (int i = 1; i < argc && argv[i][0] == '-'; i++)
  {
    switch (argv[i][1])
    {
    case 'h':
      usage();
      break;
    case 'v':
      verbose = true;
      break;
    case 'p':
      port = atoi(argv[i + 1]);
      i++;
      break;
    }
  }

  return 0;
}

