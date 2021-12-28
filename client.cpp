#pragma once
#include "client.hpp"
#include <iostream>

std::shared_ptr<AbstractProcessablePacket> AbstractProcessablePacket::getInstance(char* buffer, size_t maxlength)
 {
	packet p;
	p.initialize(buffer, maxlength);
	if (p.op == op_t::SOLVE) return std::make_shared<solveProblemPacket>(p);
	else if (p.op == op_t::CLOSE) return std::make_shared<closePacket>(p);
	else throw std::exception("unkwnown op, this should never happen!");
}

bool processmsg(char* buffer, size_t maxlength, std::queue<std::shared_ptr<AbstractProcessablePacket>>& packets) {
	for (;;) {
		packets.push(AbstractProcessablePacket::getInstance(buffer, maxlength));
		int term = packet::findTermination(buffer, maxlength);
		memcpy(buffer, buffer + term + 1, (maxlength - term - 1) * sizeof(char));
		if (packet::findTermination(buffer, maxlength) == -1) return true;
	}
}

unsigned handleClient(SOCKET sock) {

	constexpr size_t maxlength = 1000000;

	char* buffer = (char*)calloc(maxlength, sizeof(char));
	std::queue<std::shared_ptr<AbstractProcessablePacket>> packets;

	int result;
	try {
		while ((result = read(sock, buffer, maxlength) == socketResult::OK)) {
			processmsg(buffer, maxlength, packets);
			std::cout << "";

			while (!packets.empty()) {
				std::shared_ptr<AbstractProcessablePacket> pack = packets.front();
				if (pack->process(sock)) {
					free(buffer);
					closesocket(sock);
					return 0;
				}
				packets.pop();
			}
		}
		if (result == socketResult::ERR_MSG_TOO_LARGE) {
			nlohmann::json j;
			j["status"] = "msg_too_large";
			j["solution"] = nlohmann::json::object();
			write(sock, j.dump());
		}
		else if (result == socketResult::ERR_TIMEOUT) {
			nlohmann::json j;
			j["status"] = "timeout";
			j["solution"] = nlohmann::json::object();
			write(sock, j.dump());
		}
		else {
			nlohmann::json j;
			j["status"] = "network_error";
			j["solution"] = nlohmann::json::object();
			write(sock, j.dump());
		}
	}
	catch (nlohmann::json::parse_error e) {
		nlohmann::json j;
		j["status"] = "parse_error";
		j["solution"] = nlohmann::json::object();
		write(sock, j.dump());
	}
	catch (std::exception e) {
		nlohmann::json j;
		j["status"] = "bad_request";
		j["details"] = e.what();
		j["solution"] = nlohmann::json::object();
		write(sock, j.dump());
	}

	free(buffer);
	closesocket(sock);
	return 0;
}