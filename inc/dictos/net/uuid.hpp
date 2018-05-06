#pragma once

namespace ns {

using nlohmann::json;
using namespace dictos;

// Conversion hooks for Uuid to json/from json
inline void to_json(json& j, const Uuid &uuid)
{
	j = string::toString(uuid);
}

inline void from_json(const json &j, Uuid &uuid)
{
	uuid = string::fromString<Uuid>(j.get<std::string>());
}

}
