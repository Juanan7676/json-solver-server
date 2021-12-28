#include "problem.h"
#include "include\cryptopp\sha.h"
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <fstream>

#define EXEC_PATH std::string("bin\\")

namespace fs = std::filesystem;

std::string bytestohex(CryptoPP::byte* buffer, size_t length) {
	std::stringstream ss;
	ss << std::hex;
	for (size_t k = 0; k < length; k++) {
		ss << std::setw(2) << std::setfill('0') << (int)buffer[k];
	}
	return ss.str();
}

class Digest {
public:
	CryptoPP::byte* buf;
	size_t length;

	Digest(CryptoPP::byte* buf, size_t length) {
		this->buf = buf;
		this->length = length;
	}

	Digest(const Digest& that) {
		this->length = that.length;
		this->buf = new CryptoPP::byte[this->length];
		memcpy(this->buf, that.buf, this->length);
	}

	~Digest() {
		delete[] buf;
	}

	friend void swap(Digest& d1, Digest& d2) {
		using std::swap;
		swap(d1.buf, d2.buf);
		swap(d1.length, d2.length);
	}

	Digest& operator=(Digest that) {
		swap(*this, that);
		return *this;
	}

	std::string toString() {
		return bytestohex(this->buf, this->length);
	}
};

Digest sha256(void* data, size_t length) {
	CryptoPP::SHA256 algo;
	CryptoPP::byte* digest = new CryptoPP::byte[algo.DigestSize()];
	algo.CalculateDigest(digest, (CryptoPP::byte*)data, length);
	Digest ret(digest, algo.DigestSize());
	return ret;
}

json processJob(json& problem) {
	// Primero calculamos el hash
	std::string pstr = problem.dump();
	std::string digest = sha256((void*)pstr.c_str(), pstr.size()).toString();

	// Vemos si la solución está ya registrada
	fs::path dir = fs::current_path().string() + "\\" + EXEC_PATH + digest;
	if (fs::exists(dir) && fs::is_directory(dir)) { // Lo está!
		fs::path outfile = dir.string() + "\\sol.json.json";
		if (fs::exists(outfile) && fs::is_regular_file(outfile)) {
			std::ifstream f(outfile);
			json j;
			f >> j;
			f.close();
			return j;
		}
		else // Lo está pero no tenemos la solución, vamos a des-registrarla para poder procesarla de nuevo
			fs::remove_all(dir);
	}
	// Creamos un directorio para la solución y corremos el programa
	fs::create_directory(dir);
	std::ofstream out( dir.string() + "\\input.json");
	out << problem;
	out.close();

	system((EXEC_PATH + "transpopt.exe " + dir.string() + "\\input.json " + dir.string() + "\\sol.json null > " + dir.string() + "\\log.txt").c_str());

	fs::path outfile = dir.string() + "\\sol.json.json";
	if (fs::exists(outfile) && fs::is_regular_file(outfile)) { // Tenemos la solución
		std::ifstream f(outfile);
		json j;
		f >> j;
		f.close();
		return j;
	}
	else // Algo ha ocurrido
		throw std::exception("El programa de optimizacion fallo, no puedo encontrar el JSON de salida!");
}