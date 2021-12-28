#pragma once
#include <string>
#include <queue>
#include <iostream>
#include "include\json.hpp"
#include "problem.h"
#include "networking.hpp"

// Representa un paquete cualquiera, que consiste en un mensaje JSON terminado en dos saltos de línea (\\n\\n) con los siguientes campos: "op" y "body".
class packet {
public:
	std::string msg;
	size_t length;
	nlohmann::json j;
	enum class op_t { SOLVE, CLOSE } op;

	// Función auxiliar que dada una cadena y su longitud máxima encuentra el índice del primer \n\n. Devuelve dicho índice o -1 si no se encontró \n\n.
	static int findTermination(char* buffer, size_t maxlength) {
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

	// Inicializa el paquete usando el menasje recibido. Puede lanzar std::exception si el paquete no tiene el formato correcto, nlohmann::json::parse_error si el payload no es una cadena JSON válida
	void initialize(char* buffer, size_t maxlength) {
		this->length = 0;
		int term = findTermination(buffer, maxlength);
		if (term <= 0) {
			throw std::exception("Received a message with no termination (\\n\\n), dropping request");
		}
		else {
			this->length = (size_t)(term)-1;
			std::string s(buffer, this->length);
			this->msg = s;
			this->j = nlohmann::json::parse(s);
			if (this->j["op"] == "solve") this->op = op_t::SOLVE;
			else if (this->j["op"] == "close") this->op = op_t::CLOSE;
			else throw std::exception("Unknown operation received!");
		}
	}
};

// Representa un paquete que se puede procesar
class AbstractProcessablePacket : public packet {
public:
	// Procesa el paquete. Devuelve true si la conexión debe ser cerrada, false si podemos continuar escuchando
	virtual bool process(SOCKET& s) = 0;
	
	// Obtiene una instancia de esta clase usando el mensaje recibido dependiendo de la operación solicitada.
	static std::shared_ptr<AbstractProcessablePacket> getInstance(char* buffer, size_t maxlength);
};

// Paquete de resolución de problema ("solve")
class solveProblemPacket : public AbstractProcessablePacket {
public:
	nlohmann::json problem;

	// Constructor
	solveProblemPacket(packet& p) {
		this->msg = p.msg;
		this->length = p.length;
		this->j = p.j;
		this->problem = this->j["body"];
	}

	bool process(SOCKET& s) {
		try {
			nlohmann::json result = processJob(this->problem);
			nlohmann::json j;
			j["status"] = "ok";
			j["solution"] = result;
			write(s, j.dump());
			return false;
		}
		catch (std::exception) {
			nlohmann::json j;
			j["status"] = "solver_error";
			j["solution"] = nlohmann::json::object();
			write(s, j.dump());
		}

		return true;
	}
};

// Paquete de cierre de conexión ("close")
class closePacket : public AbstractProcessablePacket {
public:
	// Constructor
	closePacket(packet& p) {
		this->msg = p.msg;
		this->length = p.length;
		this->j = p.j;
	}

	bool process(SOCKET& s) {
		return true;
	}
};

// Procesa un mensaje. Recibe el mensaje recibido y una cola para introducir todos los paquetes contenidos en ese mensaje, separados por \n\n.
bool processmsg(char* buffer, size_t maxlength, std::queue<std::shared_ptr<AbstractProcessablePacket> >& packets);
// Función para procesar a un cliente. Obtiene el socket de dicho cliente.
unsigned handleClient(SOCKET s);