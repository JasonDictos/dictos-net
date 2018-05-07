#pragma once

namespace dictos::net {

inline void to_json(json& j, const Uuid& uuid) {
	j = uuid.__toString();
}

inline void from_json(const json& j, Uuid& uuid) {
	uuid = Uuid::__fromString(j.get<std::string>());
}

}
