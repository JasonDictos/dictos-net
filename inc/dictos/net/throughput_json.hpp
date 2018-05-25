#pragma once

namespace dictos::util {

// Conversion hooks for Throughput to json/from json
inline void to_json(dictos::net::json& j, const Throughput::Stats &stats)
{
	j = dictos::net::json{
		{"size", stats.size.asBytes<uint64_t>()},
		{"count", stats.count.asNumber<uint64_t>()},
		{"rate_size", stats.rateSize},
		{"rate_count", stats.rateCount},
		{"run_time", stats.runTime.count()}
	};
}

inline void from_json(const dictos::net::json& j, Throughput::Stats &stats)
{
	stats.rateCount = j.at("rate_count").get<double>();
	stats.rateSize = j.at("rate_size").get<double>();
	stats.size = j.at("size").get<uint64_t>();
	stats.count = j.at("count").get<uint64_t>();
	stats.runTime = time::seconds(j.at("run_time").get<uint32_t>());
}

}
