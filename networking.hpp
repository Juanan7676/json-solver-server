#pragma once
#include <WinSock2.h>
#include <Windows.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <string>

#define PUERTO "23840"

enum socketResult { OK, ERR_TIMEOUT, ERR_MSG_TOO_LARGE, ERR_UNKNOWN };

int read(SOCKET& s, char* buffer, size_t maxlength);
int write(SOCKET& s, std::string msg);
int startServer(SOCKET& sock);