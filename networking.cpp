#include "networking.hpp"

int findTermination(char* buffer, size_t maxlength) {
	int n = 0;
	bool strike = false;
	while (buffer[n] != 0 && n < maxlength) {
		if (buffer[n] != '\n') strike = false;
		else if (strike) return n;
		else strike = true;
		n++;
	}
	return -1;
}

int read(SOCKET& s, char* buffer, size_t maxlength) {

	size_t receivedBytes = 0;
	while (receivedBytes < maxlength) {
		int res = recv(s, buffer + receivedBytes, 256, (int)0);
		receivedBytes += res;
		if (findTermination(buffer, maxlength) != -1) return socketResult::OK;

		if (res == SOCKET_ERROR) {
			int err = WSAGetLastError();
			if (err == WSAETIMEDOUT)
				return socketResult::ERR_TIMEOUT;
			else if (err == WSAEMSGSIZE)
				return socketResult::ERR_MSG_TOO_LARGE;
			else
				return socketResult::ERR_UNKNOWN;
		}
	}
	return socketResult::ERR_MSG_TOO_LARGE;
}

int write(SOCKET& s, std::string msg) {
	int res = send(s, msg.c_str(), msg.length(), (int)0);
	if (res == SOCKET_ERROR) {
		if (WSAGetLastError() == WSAETIMEDOUT)
			return socketResult::ERR_TIMEOUT;
		else
			return socketResult::ERR_UNKNOWN;
	}
	return socketResult::OK;
}

int startServer(SOCKET &sock) {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return 1;
	}
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(NULL, PUERTO, &hints, &result) != 0) {
		WSACleanup();
		return 2;
	}
	if ((sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) == INVALID_SOCKET) {
		freeaddrinfo(result);
		WSACleanup();
		return 3;
	}
	if (bind(sock, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
		freeaddrinfo(result);
		closesocket(sock);
		WSACleanup();
		return 4;
	}
	freeaddrinfo(result);
	if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(sock);
		WSACleanup();
		return 5;
	}

	return 0;
}
