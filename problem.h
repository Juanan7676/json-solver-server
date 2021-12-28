#pragma once
#include "include\json.hpp"

using json = nlohmann::json;

// recibe un problema en formato json, lo procesa y devuelve la solución en json. Puede lanzar std::exception si ocurre algún error.
json processJob(json& problem);