#pragma once

namespace ns {

using nlohmann::json;
using namespace dictos;

// Conversion hooks for Uuid to json/from json
inline void to_json(json& j, const memory::Heap &heap)
{
	j = string::toString(heap);
}

}
