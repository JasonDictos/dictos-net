#pragma once

namespace {

using TYPE = dictos::net::protocol::TYPE;

std::map<std::string, TYPE> &PrefixMap()
{
	static std::map<std::string, TYPE> prefixMap = {
		{"tcp", TYPE::Tcp},
		{"tcpv4", TYPE::Tcp},
		{"tcpv6", TYPE::Tcp},
		{"ssl", TYPE::Ssl},
		{"tls", TYPE::Ssl},
		{"ws", TYPE::WebSocket},
		{"wss", TYPE::SslWebSocket},
		{"udp", TYPE::Udp},
		{"unix", TYPE::UnixDomain},
		{"file", TYPE::File},
		{"pipe", TYPE::Pipe},
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
