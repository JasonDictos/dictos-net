#pragma once

namespace dictos::util {

inline void to_json(net::json& j, const Throughput::Stats &stats) {
	j = stats.__toString();
}

}
