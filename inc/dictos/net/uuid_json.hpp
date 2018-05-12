#pragma once

namespace dictos {

inline void to_json(net::json& j, const Uuid& uuid) {
	j = uuid.__toString();
}

inline void from_json(const net::json& j, Uuid& uuid) {
	uuid = Uuid::__fromString(j.get<std::string>());
}

}
