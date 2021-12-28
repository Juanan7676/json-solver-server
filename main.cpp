#include <iostream>
#include <sstream>
#include "client.hpp"
#include <thread>

void initialize() {

}

int main() {

	SOCKET s;
	startServer(s);

	while (true) {
		SOCKET clisock = accept(s, NULL, NULL);
		if (clisock == INVALID_SOCKET)
			std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
		else {
			std::cout << "Accepted client" << std::endl;
			std::thread clientThread(handleClient, clisock);
			clientThread.detach();
		}
	}

	return 0;
}
