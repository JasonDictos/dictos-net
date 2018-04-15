#pragma once

namespace {

using TYPE = dictos::net::protocol::TYPE;

std::map<std::string, TYPE> &PrefixMap()
{
	static std::map<std::string, TYPE> prefixMap = {
		{"tcp", TYPE::TCPv4},
		{"tcpv4", TYPE::TCPv4},
		{"udp", TYPE::UDP},
		{"unix", TYPE::UnixDomain},
		{"file", TYPE::File},
		{"pipe", TYPE::Pipe},
		{"tcpv6", TYPE::TCPv6},
	};

	return prefixMap;
}

}

namespace dictos::net::protocol {

inline TYPE lookup(const std::string &prefix)
{
	auto iter = PrefixMap().find(prefix);
	if (iter == PrefixMap().end())
		DCORE_THROW(NotFound, "Invalid protocol prefix:", prefix);
	return iter->second;
}

}
