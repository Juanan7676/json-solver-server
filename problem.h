#pragma once
#include "include\json.hpp"

using json = nlohmann::json;

// recibe un problema en formato json, lo procesa y devuelve la soluci�n en json. Puede lanzar std::exception si ocurre alg�n error.
json processJob(json& problem);